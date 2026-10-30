// Laberint AF: animaciones, salas con jefes y cofres mejores, partidas que empiezan de cero, personalización y habilidades de los
// cazadores (pedido de Facu, octubre 2026). Uso: node tests/t-jefes.mjs  (capturas en .salidas/umbral-jefes/)
import fs from 'node:fs';import path from 'node:path';import {need,ROOT,OUT} from '../tools/dep.mjs';const {chromium}=need('playwright');
const DIR=path.join(OUT,'umbral-jefes');fs.mkdirSync(DIR,{recursive:true});
let fails=0;const ok=(c,m)=>{console.log((c?'  ✓ ':'  ✗ ')+m);if(!c)fails++};
const b=await chromium.launch();const errs=[];const pg=await b.newPage({viewport:{width:1400,height:860}});
pg.on('pageerror',e=>errs.push(e.message));await pg.route(/fonts\.g/,r=>r.abort());
await pg.addInitScript(()=>{let a=11;Math.random=()=>{a=a+0x6D2B79F5|0;let t=Math.imul(a^a>>>15,1|a);t=t+Math.imul(t^t>>>7,61|t)^t;return((t^t>>>14)>>>0)/4294967296};localStorage.setItem('umbral_alijo',JSON.stringify({gold:900,up:{vida:3,mochila:2},chars:['nadie','vigia'],runs:3,escapes:1,ach:[]}))});
await pg.goto('file://'+path.join(ROOT,'index.html'));await pg.waitForTimeout(500);
const U=(f,a)=>pg.evaluate(f,a);
// ---- personalización en el menú, con vista previa
ok(await U(()=>!/ORO/.test(document.getElementById('card').textContent.replace(/BÓVEDAS|oro, armas/g,''))&&!document.querySelector('[data-u],[data-l]')),'el menú no tiene alijo de oro, equipo para comprar ni mejoras');
for(const [k,v] of [['hs','afro'],['barba','barba'],['acc','cuernos'],['capa','2'],['skin','4'],['hair','7'],['col','5']])await pg.click(`[data-lk="${k}"][data-v="${v}"]`);
const lk=await U(()=>JSON.parse(localStorage.getItem('umbral_alijo')));
ok(lk.look.hs==='afro'&&lk.look.barba==='barba'&&lk.look.acc==='cuernos'&&lk.look.capa===2&&lk.look.skin===4&&lk.look.hair===7&&lk.look.col===5&&lk.gold===undefined,'se guarda el personaje (peinado, barba, accesorio, capa, piel, pelo, abrigo) y nada de oro');
await pg.waitForTimeout(300);
const pv=await U(()=>{const c=document.getElementById('lookCv'),d=c.getContext('2d').getImageData(0,0,c.width,c.height).data;let n=0;for(let i=3;i<d.length;i+=4)if(d[i]>0)n++;return n});
ok(pv>2000,'la vista previa dibuja al personaje ('+pv+' píxeles)');
await pg.locator('.stash.look').screenshot({path:path.join(DIR,'0-personaje.png')});
await pg.click('#startBtn');await pg.waitForTimeout(250);await U(()=>{const i=document.getElementById('intro');if(!i.hidden)i.click()});
ok(await U(()=>{const P=__umbral.FIG.p0.P,h=__umbral.human;return P.hs==='afro'&&P.barba==='barba'&&P.acc==='cuernos'&&!!P.cape&&h.gold===0&&h.maxhp===100&&h.bag.length===12}),'la partida arranca de cero (sin oro, 100 de vida, mochila de 12) con tu personaje');
// ---- farmeo: los primeros 2 minutos no hay cazadores
const f0=await U(()=>{const U=__umbral;U.step(.05);return{dorm:U.hunters.every(h=>h.dorm&&!h.alive),mobs:U.mobs.filter(m=>m.alive&&!U.isBoss(m)).length,hud:document.getElementById('huntLine').textContent,intro:document.getElementById('introLv').textContent}});
ok(f0.dorm&&f0.mobs>40&&/FARMEO/.test(f0.hud)&&/SALE EN 2:00/.test(f0.intro),`al arrancar es puro farmeo: sin cazadores, ${f0.mobs} bestias y el HUD dice «${f0.hud}»`);
const f1=await U(()=>{const U=__umbral;U.time=U.FARM-.1;U.step(.05);const a=U.hunters.filter(h=>h.dorm).length;U.step(.1);U.step(.05);
  const near=U.hunters.filter(h=>h.alive).map(h=>Math.hypot(h.x-h.target.x,h.y-h.target.y));const pack={};for(const h of U.hunters)if(h.type==='perro'||h.type==='duende')(pack[h.target.id+h.type]=pack[h.target.id+h.type]||[]).push(h);
  const spread=Math.max(0,...Object.values(pack).map(L=>Math.max(...L.map(h=>Math.hypot(h.x-L[0].x,h.y-L[0].y)))));
  return{before:a,out:U.farmOut,alive:U.hunters.filter(h=>h.alive).length,all:U.hunters.length,min:Math.min(...near),spread,ev:document.getElementById('evT').textContent}});
ok(f1.before===f1.all&&f1.out&&f1.alive===f1.all&&f1.min>150&&f1.spread<40&&/SALEN LOS CAZADORES/.test(f1.ev),`a los 2 minutos salen los ${f1.alive} cazadores lejos de su presa (el más cerca a ${Math.round(f1.min)}) y las jaurías juntas, con aviso «${f1.ev}»`);
// ---- salas
const rm=await U(()=>{const U=__umbral;return{boss:U.rooms.filter(r=>r.type==='boss').length,vault:U.rooms.filter(r=>r.type==='vault').length,bosses:U.mobs.filter(U.isBoss).map(m=>m.type),elites:U.mobs.filter(m=>m.elite).length,oro:U.chests.filter(c=>c.tier==='oro').length,ley:U.chests.filter(c=>c.tier==='leyenda'&&c.sealed).length}});
ok(rm.boss>=1&&rm.bosses.length===rm.boss&&new Set(rm.bosses).size===rm.bosses.length,`salas de jefe con jefes distintos: ${rm.bosses.join(', ')}`);
ok(rm.vault>=1&&rm.oro===rm.vault&&rm.elites===rm.vault*3,`bóvedas con cofre de oro y 3 bestias de élite cada una (${rm.vault} bóvedas, ${rm.elites} élites)`);
ok(rm.ley===rm.boss,'cada sala de jefe tiene su cofre legendario sellado');
// escena limpia en la sala de la salida
await U(()=>{const U=__umbral,h=U.human;for(const m of U.mobs)m.alive=false;for(const x of U.hunters){x.alive=false;x.dorm=false};for(const p of U.players)if(p!==h)p.alive=false;U.time=30;
  const r=U.rooms.find(r=>r.type==='exit');window.__C={x:r.x-26,y:r.y};h.x=r.x-26;h.y=r.y;h.hp=h.maxhp=1e5;h.aim=0;U.drops.length=0;U.step(.02)});
// ---- cofres
const ch=await U(()=>{const U=__umbral,h=U.human,c=U.chests.find(c=>c.tier==='leyenda'),m=c.sealed;const o={x:c.x,y:c.y};m.alive=true;c.x=h.x+6;c.y=h.y;U.drops.length=0;U.openChest(h,c);const closed=!c.open;
  m.alive=false;U.openChest(h,c);const items=U.drops.filter(d=>d.type==='item').map(d=>d.def.rar||0);c.x=o.x;c.y=o.y;
  const g=U.chests.find(c=>c.tier==='oro');g.x=h.x+6;g.y=h.y;U.drops.length=0;const g0=h.gold;U.openChest(h,g);const it2=U.drops.filter(d=>d.type==='item').map(d=>d.def.rar||0);return{closed,open:c.open,items,it2,gold:h.gold-g0}});
ok(ch.closed,'el cofre legendario no se abre mientras su jefe vive');
ok(ch.open&&ch.items.includes(3),'muerto el jefe, el cofre legendario suelta un arma legendaria ('+ch.items.join(',')+')');
ok(ch.it2.length===2&&ch.it2.every(r=>r>=2)&&ch.gold>=80,'el cofre de oro trae dos cosas épicas o mejores y oro ('+ch.it2.join(',')+', '+ch.gold+' de oro)');
// ---- animaciones
const an=await U(()=>{const U=__umbral,h=U.human,P=U.pose;const o={};U.drops.length=0;
  const p0=P(h);h.cd=0;h.queued='normal';h.queueT=.35;U.step(.03);o.swing=P(h).fx>p0.fx+.5;
  const z=U.mkMob('zombi',{x:h.x+30,y:h.y});U.mobs.push(z);U.step(.01);z.hp=1e4;U.step(.01);
  h.swingT=0;h.iframe=0;const hp=h.hp;z.x=h.x+8;z.y=h.y;z.aggro=h;z.cd=0;let wind=false,atk=false;for(let i=0;i<30;i++){U.step(.03);if(z.windT>0)wind=wind||P(z).fx<-.5;if(z.atkT>0)atk=atk||P(z).fx>1}o.wind=wind;o.atk=atk;o.hurt=h.hp<hp;z.alive=false;
  h.throws.bomba=1;h.tsel='bomba';h.bsel='bomba';U.useBelt();o.throw=h.throwT>0;
  h.hp=h.maxhp-100;h.potions=1;h.bsel='potion';U.useBelt();o.drink=h.drinkT>0;
  return o});
ok(an.swing,'al pegar el cuerpo hace una estocada hacia adelante');
ok(an.wind&&an.atk,'las bestias se echan atrás antes del golpe y se tiran al pegar');
ok(an.throw&&an.drink,'animación de tirar (bomba) y de tomar (poción)');
// ---- jefes
const BS={};
for(const type of ['jefe','carnicero','bruja']){
  const r=await U(type=>{const U=__umbral,h=U.human,C=window.__C;for(const m of U.mobs)m.alive=false;U.teles.length=0;U.areas.length=0;U.projs.length=0;h.x=C.x;h.y=C.y;h.st.stun=h.st.root=0;
    const m=U.mkMob(type,{x:h.x+50,y:h.y});m.aggro=h;U.mobs.push(m);window.__m=m;const used=new Set(),seen={tele:0,cast:0,bar:false};
    for(let i=0;i<180;i++){h.hp=h.maxhp;h.x=C.x;h.y=C.y;if(m.aggro!==h)m.aggro=h;U.step(.05);for(const k in m.acd)used.add(k);if(U.teles.some(t=>t.src===m&&t.k!=='none'))seen.tele++;if(m.castT>0)seen.cast++;if(!document.getElementById('bossBar').hidden)seen.bar=true}
    m.hp=m.maxhp*.4;for(let i=0;i<10;i++){m.aggro=h;U.step(.05)}
    return{used:[...used],tele:seen.tele,cast:seen.cast,bar:seen.bar,p2:!!m.p2,name:document.getElementById('bossN').textContent}},type);
  BS[type]=r;
  ok(r.used.length>=2&&r.tele>0&&r.cast>0,`${type}: usa sus habilidades (${r.used.join(', ')}), con aviso en el piso y animación`);
  ok(r.bar&&r.p2&&/FURIOSO/.test(r.name),`${type}: barra del jefe arriba y se enfurece a la mitad de la vida («${r.name}»)`);
  if(type==='carnicero'){await U(()=>{for(let i=0;i<4;i++)__umbral.step(.05)});await pg.screenshot({path:path.join(DIR,'1-jefe.png')})}
}
// ---- habilidades de los cazadores
const NEW={roberto:['slug','cepo'],duende:['polvo','vanish'],vibora:['spit','lunge'],perro:['howl','pounce'],arana:['jump'],ninja:['shadow','fan'],gigante:['rock','charge'],bob:['mist','drain'],verase:['cone','snakes'],diablo:['meteor','wall']};
for(const type of Object.keys(NEW)){
  const r=await U(([type,keys])=>{const U=__umbral,h=U.human,C=window.__C;for(const m of U.mobs)m.alive=false;for(const x of U.hunters){x.alive=false;x.dorm=false};U.teles.length=0;U.areas.length=0;U.projs.length=0;
    const x=U.hunters.find(x=>x.type===type&&(type!=='perro'||x.variant==='alfa'));x.alive=true;x.hp=x.maxhp=1e5;x.target=h;x.acd={};x.stunT=0;x.leap=null;x.dash=null;x.lock=0;x.x=h.x+(type==='duende'?30:70);x.y=h.y;
    const used=new Set();for(let i=0;i<200;i++){h.hp=h.maxhp;h.x=C.x;h.y=C.y;h.st.stun=h.st.root=0;h.iframe=0;x.aw=1;x.det=true;if(type==='duende'&&i===60)x.fleeT=3;U.step(.05);for(const k in x.acd)if(keys.includes(k))used.add(k);if(used.size===keys.length)break}
    return [...used]},[type,NEW[type]]);
  ok(r.length>0,`${type}: habilidades nuevas en uso (${r.join(', ')||'ninguna'})`);
}
// Verase: si la mirás quedás de piedra; de espaldas no
const vr=await U(()=>{const U=__umbral,h=U.human,C=window.__C,v=U.hunters.find(x=>x.type==='verase');for(const x of U.hunters){x.alive=false;x.dorm=false};v.alive=true;v.target=h;const res=[];
  for(const look of [true,false]){U.teles.length=0;h.x=C.x;h.y=C.y;h.st.stun=0;v.x=h.x+60;v.y=h.y;v.acd={};v.lock=0;v.aim=Math.PI;U.hunterSkills(v,h,60,true,0);const T=U.teles.find(t=>t.k==='cone');
    for(let i=0;i<30&&U.teles.includes(T);i++){h.aim=look?0:Math.PI;h.x=C.x;h.y=C.y;v.x=h.x+60;v.y=h.y;v.lock=1;U.step(.05)}res.push(h.st.stun)}return res});
ok(vr[0]>1&&vr[1]<=0,`Verase: mirándola quedás de piedra (${vr[0].toFixed(1)} s); de espaldas, no (${vr[1].toFixed(1)} s)`);
// ---- partida sin nada que sacar
const nc=await U(()=>{const U=__umbral,h=U.human;h.gold=777;h.keys.add(h.id);U.extract(h);for(let i=0;i<60;i++)U.step(.05);return{st:U.state,store:JSON.parse(localStorage.getItem('umbral_alijo'))}});
ok(nc.st==='end'&&nc.store.gold===undefined&&nc.store.escapes===2,'al escapar con oro no se guarda nada para la próxima (solo la cuenta de escapes)');
await pg.click('#rematchBtn');await pg.waitForTimeout(250);
ok(await U(()=>__umbral.human.gold===0&&__umbral.human.bar.filter(Boolean).length===1),'la partida siguiente empieza otra vez de cero');
ok(!errs.length,'sin errores'+(errs.length?': '+[...new Set(errs)].slice(0,3).join(' | '):''));
await b.close();console.log('Capturas en '+DIR);console.log(fails?'FALLAS: '+fails:'TODO OK');process.exit(fails?1:0);
