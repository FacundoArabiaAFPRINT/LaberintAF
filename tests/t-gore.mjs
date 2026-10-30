// Laberint AF: gore (pedido de Facu, octubre 2026). Partes del cuerpo que se desprenden: una pierna menos va más lento, un brazo menos
// pega más flojo; el muñón sangra y deja rastro; los muy heridos dejan gotas; al morir el cuerpo vuela en pedazos.
// Uso: node tests/t-gore.mjs  (capturas en .salidas/umbral-gore/)
import fs from 'node:fs';import path from 'node:path';import {need,ROOT,OUT} from '../tools/dep.mjs';const {chromium}=need('playwright');
const DIR=path.join(OUT,'umbral-gore');fs.mkdirSync(DIR,{recursive:true});
let fails=0;const ok=(c,m)=>{console.log((c?'  ✓ ':'  ✗ ')+m);if(!c)fails++};
const b=await chromium.launch();const pg=await b.newPage({viewport:{width:1400,height:860}});const errs=[];
pg.on('pageerror',e=>errs.push(e.message));await pg.route(/fonts\.g/,r=>r.abort());
await pg.addInitScript(()=>{let a=11;Math.random=()=>{a=a+0x6D2B79F5|0;let t=Math.imul(a^a>>>15,1|a);t=t+Math.imul(t^t>>>7,61|t)^t;return((t^t>>>14)>>>0)/4294967296}});
await pg.goto('file://'+path.join(ROOT,'index.html'));await pg.waitForTimeout(500);
await pg.click('#startBtn');await pg.waitForTimeout(300);await pg.evaluate(()=>{const i=document.getElementById('intro');if(!i.hidden)i.click()});
const U=(f,a)=>pg.evaluate(f,a);
// a la sala más grande, sin enemigos que molesten
await U(()=>{const U=__umbral,h=U.human,r=U.rooms.filter(r=>r.type==='hall'||r.type==='boss').sort((a,b)=>(b.x1-b.x0)*(b.y1-b.y0)-(a.x1-a.x0)*(a.y1-a.y0))[0];h.x=r.x;h.y=r.y;h.hp=h.maxhp=100000;for(const m of U.mobs)m.alive=false;for(const x of U.hunters){x.alive=false;x.dorm=false}});
const run=async()=>{const x0=await U(()=>__umbral.human.x);await pg.keyboard.down('KeyD');await U(()=>{for(let i=0;i<10;i++)__umbral.step(0.03)});await pg.keyboard.up('KeyD');const x1=await U(()=>__umbral.human.x);await pg.keyboard.down('KeyA');await U(()=>{for(let i=0;i<10;i++)__umbral.step(0.03)});await pg.keyboard.up('KeyA');return x1-x0};
const v0=await run();
await U(()=>__umbral.sever(__umbral.human,'legR'));const v1=await run();
const lm=await U(()=>__umbral.legMul(__umbral.human));
ok(lm<=.6&&v1<v0&&v1>0,`sin una pierna va más lento (velocidad ×${lm}; recorrido ${v0.toFixed(1)} → ${v1.toFixed(1)})`);
ok(await U(()=>__umbral.gibs.some(g=>g.kind==='leg')),'la pierna sale volando como resto');
ok(await U(()=>__umbral.armMul(__umbral.human)===1),'con los dos brazos pega entero');
await U(()=>__umbral.sever(__umbral.human,'armR'));
ok(await U(()=>__umbral.armMul(__umbral.human)<.7),'sin un brazo pega más flojo');
ok(await U(()=>/sin/.test(document.getElementById('inv').textContent)),'el HUD dice lo que le falta: '+await U(()=>document.getElementById('inv').textContent.slice(0,60)));
// sangrado: resta vida y deja rastro
const d0=await U(()=>{const h=__umbral.human;h.maxhp=100;h.hp=90;h.bleed=6;h.iframe=0;return __umbral.decals.length});
await run();const st=await U(()=>({hp:__umbral.human.hp,dec:__umbral.decals.length}));
ok(st.hp<90,'el muñón sangra y resta vida ('+st.hp.toFixed(1)+')');
ok(st.dec>d0+3,`deja rastro de sangre al caminar (${st.dec-d0} gotas)`);
await U(()=>{__umbral.human.potions=1;__umbral.human.hp=60});await pg.keyboard.press('KeyQ');
ok(await U(()=>!(__umbral.human.bleed>0)),'una poción corta el sangrado');
await pg.screenshot({path:path.join(DIR,'1-jugador.png'),clip:{x:450,y:230,width:500,height:400}});
// un enemigo pierde una pata o una pierna: más lento; un brazo: menos daño
const en=await U(()=>{const U=__umbral,h=U.human,z=U.mkMob('zombi',{x:h.x+30,y:h.y});U.mobs.push(z);z.hp=z.maxhp;z.x=h.x+30;z.y=h.y;const s0=z.speed,d0=z.dmg;U.sever(z,'legL');U.sever(z,'armL');return{s0,s1:z.speed,d0,d1:z.dmg,t:z.type}});
ok(en.s1<en.s0&&en.d1<en.d0,`un ${en.t} sin pierna y sin brazo: velocidad ${en.s0}→${en.s1.toFixed(1)}, daño ${en.d0}→${en.d1.toFixed(1)}`);
const dog=await U(()=>{const U=__umbral,h=U.human,d=U.mkMob('perro',{x:h.x-30,y:h.y+10});U.mobs.push(d);d.hp=d.maxhp;d.x=h.x-30;d.y=h.y+10;const s0=d.speed;U.sever(d,'armR');return{s0,s1:d.speed}});
ok(!dog||dog.s1<dog.s0,'un perro sin una pata va más lento');
// cortes en combate: golpes de espada a un zombi duro hasta que pierde algo
const cut=await U(()=>{const U=__umbral,h=U.human,z=U.mkMob('zombi',{x:h.x+10,y:h.y});U.mobs.push(z);z.maxhp=z.hp=60;z.x=h.x+10;z.y=h.y;h.bar[0]=U.IT.espada;window.__z=z;return true});
let lost=false;if(cut){for(let i=0;i<40&&!lost;i++){await U(()=>{const U=__umbral,h=U.human,z=window.__z;z.hp=z.maxhp;z.x=h.x+10;z.y=h.y;h.aim=0;h.cd=0});await pg.keyboard.down('KeyJ');await U(()=>__umbral.step(0.03));await pg.keyboard.up('KeyJ');await U(()=>{for(let k=0;k<3;k++){__umbral.human.aim=0;__umbral.step(0.03)}});lost=await U(()=>!!(window.__z.limbs&&Object.values(window.__z.limbs).includes(false)))}}
ok(lost,'a espadazos, un zombi termina perdiendo un miembro');
// al morir vuela en pedazos
const g0=await U(()=>__umbral.gibs.length);
await U(()=>{const U=__umbral,h=U.human,v=U.players.find(p=>p!==h&&p.alive);v.x=h.x+20;v.y=h.y+18;v.hp=1;v.iframe=0;window.__v=v});
await U(()=>{const v=window.__v;v.hp=-1;});await U(()=>{const U=__umbral,v=window.__v;v.hp=5;v.st.burn=4;for(let i=0;i<40&&v.alive;i++)U.step(0.05)});
await U(()=>{for(let i=0;i<20;i++)__umbral.step(0.03)});
const g1=await U(()=>({n:__umbral.gibs.length,k:[...new Set(__umbral.gibs.map(g=>g.kind))]}));
ok(g1.n>g0+2&&g1.k.includes('torso'),'al morir deja el torso y pedazos: '+g1.k.join(', '));
await pg.screenshot({path:path.join(DIR,'2-restos.png'),clip:{x:450,y:230,width:500,height:400}});
ok(!errs.length,'sin errores'+(errs.length?': '+[...new Set(errs)].slice(0,3).join(' | '):''));
await b.close();console.log(fails?'FALLAS: '+fails:'TODO OK');process.exit(fails?1:0);
