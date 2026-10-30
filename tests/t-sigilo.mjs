// Laberint AF: sigilo y lo que vino con él (pedido de Facu, octubre 2026): agacharse, arbustos, enemigos que te detectan («?» y «!»),
// ruido, cazadores que te buscan, vendas y prótesis, pausa y opciones, mejoras y personajes, biomas y clima, armas que se
// gastan y recarga, muerte en cámara lenta y escape. Uso: node tests/t-sigilo.mjs  (capturas en .salidas/umbral-sigilo/)
import fs from 'node:fs';import path from 'node:path';import {need,ROOT,OUT} from '../tools/dep.mjs';const {chromium}=need('playwright');
const DIR=path.join(OUT,'umbral-sigilo');fs.mkdirSync(DIR,{recursive:true});
let fails=0;const ok=(c,m)=>{console.log((c?'  ✓ ':'  ✗ ')+m);if(!c)fails++};
const b=await chromium.launch();const errs=[];
const page=async(pre)=>{const pg=await b.newPage({viewport:{width:1400,height:860}});pg.on('pageerror',e=>errs.push(e.message));await pg.route(/fonts\.g/,r=>r.abort());
  await pg.addInitScript(pre=>{let a=5;Math.random=()=>{a=a+0x6D2B79F5|0;let t=Math.imul(a^a>>>15,1|a);t=t+Math.imul(t^t>>>7,61|t)^t;return((t^t>>>14)>>>0)/4294967296};if(pre)for(const k in pre)localStorage.setItem(k,pre[k])},pre||null);
  await pg.goto('file://'+path.join(ROOT,'index.html'));await pg.waitForTimeout(400);return pg};
let pg=await page({umbral_alijo:JSON.stringify({gold:5000,up:{vida:2},runs:0,escapes:0})});
const U=(f,a)=>pg.evaluate(f,a);
// habilidad desde el menú (cada partida empieza de cero: el oro y las mejoras viejas se ignoran)
await pg.click('[data-c="corredora"]');
const st=await U(()=>JSON.parse(localStorage.getItem('umbral_alijo')));
ok(st.char==='corredora'&&st.gold===undefined&&st.up===undefined,'la habilidad se elige gratis y no queda oro ni mejoras guardadas');
await pg.click('#startBtn');await pg.waitForTimeout(250);await U(()=>{const i=document.getElementById('intro');if(!i.hidden)i.click()});
ok(await U(()=>__umbral.human.maxhp===100&&__umbral.human.gold===0&&__umbral.human.char==='corredora'),'la partida arranca de cero (100 de vida, sin oro) con la habilidad elegida');
ok(await U(()=>['bosque','cripta','pantano','nieve'].includes(__umbral.biome)),'cada noche tiene bioma: '+await U(()=>__umbral.biome));
ok(await U(()=>__umbral.bushes.length>30),'hay arbustos para esconderse: '+await U(()=>__umbral.bushes.length));
// a un arbusto, sin nadie cerca
await U(()=>{const U=__umbral,h=U.human;for(const m of U.mobs)m.alive=false;for(const x of U.hunters){x.alive=false;x.dorm=false};for(const p of U.players)if(p!==h){p.alive=false}
  const bu=U.bushes.find(b=>!U.bushes.some(o=>o!==b&&Math.hypot(o.x-b.x,o.y-b.y)<30))||U.bushes[0];h.x=bu.x;h.y=bu.y;h.hp=h.maxhp=100000;window.__bu=bu;U.step(.02)});
const v0=await U(()=>__umbral.human.sv);await pg.keyboard.press('KeyC');await U(()=>__umbral.step(.02));const v1=await U(()=>__umbral.human.sv);
ok(await U(()=>__umbral.human.crouch)&&v1<v0&&v1<.25,`agachado en el arbusto casi no te ven (visibilidad ${v0.toFixed(2)} → ${v1.toFixed(2)})`);
ok(await U(()=>document.getElementById('stl').textContent)==='OCULTO','el HUD dice OCULTO');
// un zombi cerca: escondido no te descubre; de pie a la vista sí
const det=await U(()=>{const U=__umbral,h=U.human,z=U.mkMob('zombi',{x:h.x+38,y:h.y});if(!U.bushAt(h.x,h.y))return null;z.aggroR=80;U.mobs.push(z);window.__z=z;for(let i=0;i<40;i++){z.x=h.x+38;z.y=h.y;U.step(.05)}return{aw:z.aw||0,ag:z.aggro===h}});
ok(det&&!det.ag&&det.aw<1,'escondido y agachado, el zombi de al lado no te descubre (atención '+(det&&det.aw.toFixed(2))+')');
await pg.keyboard.press('KeyC');
await U(()=>{const U=__umbral,h=U.human,r=U.rooms.find(r=>r.type==='exit');h.x=r.x-30;h.y=r.y;h.light=true;h.oil=100;U.step(.02)});
const seen=await U(()=>{const U=__umbral,h=U.human,z=window.__z;let q=false;for(let i=0;i<50;i++){z.x=h.x+40;z.y=h.y;U.step(.05);if((z.aw||0)>.1&&(z.aw||0)<1)q=true;if(z.det)break}return{q,det:!!z.det,ag:z.aggro===h}});
ok(seen.q,'antes de descubrirte, el enemigo muestra el «?» que se va llenando');
ok(seen.det&&seen.ag,'de pie y a la vista, te descubre («!») y te persigue');
await pg.screenshot({path:path.join(DIR,'1-detectado.png'),clip:{x:450,y:230,width:500,height:400}});
// ruido: un perro lejos y fuera de la vista se pone en alerta cuando corrés
const nz=await U(()=>{const U=__umbral,h=U.human,d=U.mkMob('perro',{x:h.x,y:h.y+100});U.mobs.push(d);window.__z.alive=false;U.noise(h.x,h.y,120,h);return{aw:d.aw||0,sus:!!d.sus}});
ok(nz.aw>=.5&&nz.sus,'el ruido pone en alerta a los que están cerca y los manda a mirar');
// cazador: te busca por la zona sin saber exactamente dónde estás
const hu=await U(()=>{const U=__umbral,h=U.human,c=h.hunter;U.time=Math.max(U.time,61);c.alive=true;c.hp=c.maxhp;c.x=h.x+260;c.y=h.y;c.aw=0;c.det=false;U.step(.05);for(let i=0;i<20;i++)U.step(.05);return{g:!!c.guess,aw:c.aw||0}});
ok(hu.g&&hu.aw<1,'el cazador que no te vio te busca por la zona aproximada');
// vendas y prótesis
await U(()=>{const U=__umbral,h=U.human;h.hunter.alive=false;h.maxhp=100;h.hp=60;h.bleed=6;h.vendas=1;h.bsel='venda';U.useBelt()});
ok(await U(()=>!(__umbral.human.bleed>0)&&__umbral.human.vendas===0),'la venda corta el sangrado');
const pp=await U(()=>{const U=__umbral,h=U.human;U.sever(h,'legL');const a=U.legMul(h);U.SHOP.find(s=>s.n==='Pata de palo').buy(h);return{a,b:U.legMul(h),v:h.limbs.legL}});
ok(pp.v==='pp'&&pp.b>pp.a,`la pata de palo devuelve velocidad (×${pp.a} → ×${pp.b.toFixed(2)})`);
// armas que se gastan y revólver con tambor
const wr=await U(()=>{const U=__umbral,h=U.human,w=h.bar[0],d0=w.dur;return{d0,max:w.durMax}});
ok(wr.d0>0&&wr.max>0,'las armas tienen estado: daga '+wr.d0+'/'+wr.max);
await U(()=>{const U=__umbral,h=U.human,d={type:'item',def:U.IT.revolver,x:h.x,y:h.y,amt:9};U.drops.push(d);U.step(.02)});await pg.keyboard.press('KeyE');
await U(()=>{const h=__umbral.human;h.ammo=20;const i=h.bar.findIndex(x=>x&&x.id==='revolver');window.__ri=i});const ri=await U(()=>window.__ri);await pg.keyboard.press('Digit'+(ri+1));
await pg.mouse.move(900,430);await pg.mouse.down();await U(()=>{for(let i=0;i<4;i++)__umbral.step(.05)});await pg.mouse.up();
ok(await U(()=>__umbral.human.reloadT>0||(__umbral.human.ranged.inMag>0)),'el revólver vacío se recarga antes de disparar');
await U(()=>{for(let i=0;i<40;i++)__umbral.step(.05)});const mg=await U(()=>__umbral.human.ranged.inMag);
ok(mg===6,'después de recargar tiene 6 en el tambor ('+mg+')');
const brk=await U(()=>{const U=__umbral,h=U.human,r=U.rooms.find(r=>r.type==='exit');h.x=r.x-30;h.y=r.y;const w=h.bar[0];w.dur=1;const z=U.mkMob('zombi',{x:h.x+10,y:h.y});z.hp=z.maxhp=999;U.mobs.push(z);h.wsel=0;h.aim=0;h.cd=0;window.__w=w;window.__zz=z;return true});
await pg.keyboard.press('Digit1');for(let i=0;i<6;i++){await U(()=>{const h=__umbral.human,z=window.__zz;z.x=h.x+10;z.y=h.y;h.aim=0;h.cd=0});await pg.keyboard.down('KeyJ');await U(()=>__umbral.step(.03));await pg.keyboard.up('KeyJ');await U(()=>{for(let k=0;k<3;k++){__umbral.human.aim=0;__umbral.step(.03)}});if(await U(()=>!__umbral.human.bar.includes(window.__w)))break}
ok(await U(()=>!__umbral.human.bar.includes(window.__w)),'un arma gastada se rompe y sale de la barra');
// clima: la lluvia lava la sangre
const rn=await U(()=>{const U=__umbral,h=U.human;for(let i=0;i<60;i++)U.decals.push({x:h.x+i,y:h.y,r:1,a:0,col:'#6a1410'});const n0=U.decals.length;U.EVENTS.lluvia.go();return{n0,n1:U.decals.length,r:U.wx.rain}});
ok(rn.r>0&&rn.n1<rn.n0*.6,`la lluvia lava la sangre (${rn.n0} → ${rn.n1} manchas)`);
await pg.screenshot({path:path.join(DIR,'2-lluvia.png')});
// pausa y opciones
await pg.keyboard.press('Escape');await pg.waitForTimeout(100);
const t0=await U(()=>({s:__umbral.state,t:__umbral.matchT}));await pg.waitForTimeout(400);const t1=await U(()=>__umbral.matchT);
ok(t0.s==='pause'&&t1===t0.t,'ESC pausa el juego y el tiempo no corre');
await pg.screenshot({path:path.join(DIR,'3-pausa.png')});
await pg.click('#pcard [data-o="q"][data-v="baja"]');
ok(await U(()=>JSON.parse(localStorage.getItem('umbral_opts')).q==='baja'&&__umbral.OPT.q==='baja'),'las opciones (calidad baja) se guardan');
await pg.click('#pcard [data-o="q"][data-v="alta"]');await pg.keyboard.press('Escape');
ok(await U(()=>__umbral.state==='play'),'ESC vuelve al juego');
// muerte en cámara lenta
await U(()=>{const U=__umbral,h=U.human,z=window.__zz;h.hp=1;h.maxhp=100;h.iframe=0;h.limbs=null});
await U(()=>{const U=__umbral,h=U.human,z=window.__zz;U.step(.01);if(h.alive){h.hp=0.5;h.st.burn=1;for(let i=0;i<10&&h.alive;i++)U.step(.05)}});
ok(await U(()=>!!__umbral.killCam&&!__umbral.human.alive),'al morir arranca la cámara lenta con quién te mató');
await U(()=>{for(let i=0;i<6;i++)__umbral.step(.05)});await pg.screenshot({path:path.join(DIR,'4-muerte.png')});
await pg.close();
// escape
pg=await page();await pg.click('#startBtn');await pg.waitForTimeout(250);await pg.evaluate(()=>{const i=document.getElementById('intro');if(!i.hidden)i.click()});
await pg.evaluate(()=>{__umbral.extract(__umbral.human);for(let i=0;i<10;i++)__umbral.step(.05)});
ok(await pg.evaluate(()=>!!__umbral.escFx),'al escapar se ve la luz de la escotilla y «¡ESCAPASTE!»');
await pg.screenshot({path:path.join(DIR,'5-escape.png')});await pg.close();
ok(!errs.length,'sin errores'+(errs.length?': '+[...new Set(errs)].slice(0,3).join(' | '):''));
await b.close();console.log(fails?'FALLAS: '+fails:'TODO OK');process.exit(fails?1:0);
