// Laberint AF: inventario (TAB), barra de armas (1-4, rueda) y cinturón de objetos (5-0, R, X). Pedido de Facu, octubre 2026.
// Uso: node tests/t-inv.mjs  (capturas en .salidas/umbral-inv/)
import fs from 'node:fs';import path from 'node:path';import {need,ROOT,OUT} from '../tools/dep.mjs';const {chromium}=need('playwright');
const DIR=path.join(OUT,'umbral-inv');fs.mkdirSync(DIR,{recursive:true});
let fails=0;const ok=(c,m)=>{console.log((c?'  ✓ ':'  ✗ ')+m);if(!c)fails++};
const b=await chromium.launch();const pg=await b.newPage({viewport:{width:1400,height:860}});const errs=[];
pg.on('pageerror',e=>errs.push(e.message));await pg.route(/fonts\.g/,r=>r.abort());
await pg.goto('file://'+path.join(ROOT,'index.html'));await pg.waitForTimeout(500);
await pg.click('#startBtn');await pg.waitForTimeout(300);await pg.evaluate(()=>{const i=document.getElementById('intro');if(!i.hidden)i.click()});
const U=(f,a)=>pg.evaluate(f,a);
await U(()=>{const h=__umbral.human;h.hp=h.maxhp=99999;for(const m of __umbral.mobs)m.alive=false;for(const x of __umbral.hunters){x.alive=false;x.dorm=false}});
ok(await U(()=>document.querySelectorAll('#wbar .slot').length===4&&document.querySelectorAll('#belt .slot').length===6),'barra de 4 armas y cinturón de 6 objetos en pantalla');
// juntar armas: van a la barra y después a la mochila
const put=async id=>{await U(id=>{const U=__umbral,h=U.human,d={type:'item',def:U.IT[id],x:h.x,y:h.y,amt:6};U.drops.push(d);U.step(0.02)},id);await pg.keyboard.press('KeyE');await U(()=>__umbral.step(0.02))};
for(const id of['espada','arco','hacha'])await put(id);
let st=await U(()=>{const h=__umbral.human;return{bar:h.bar.map(x=>x&&x.id),bag:h.bag.filter(Boolean).map(x=>x.id),ammo:h.ammo}});
ok(st.bar.join()==='daga,espada,arco,hacha','las armas que se juntan van a la barra: '+st.bar.join());
await put('katana');await put('linterna');
st=await U(()=>{const h=__umbral.human;return{bag:h.bag.filter(Boolean).map(x=>x.id),lamp:h.lamp.id}});
ok(st.bag.includes('katana'),'con la barra llena, el arma va a la mochila');
ok(st.lamp==='linterna'&&st.bag.includes('farol'),'una luz mejor se pone sola y la vieja va a la mochila');
// elegir arma con las teclas y con la rueda
await pg.keyboard.press('Digit3');await U(()=>__umbral.step(0.02));
st=await U(()=>{const h=__umbral.human;return{w:h.wsel,r:h.ranged&&h.ranged.id}});
ok(st.w===2&&st.r==='arco','la tecla 3 pone el arco en la mano');
const a0=await U(()=>__umbral.human.ammo);await pg.mouse.move(900,430);await pg.mouse.down();await U(()=>{for(let i=0;i<4;i++)__umbral.step(0.05)});await pg.mouse.up();
ok(await U(()=>__umbral.human.ammo)<a0,'con un arma a distancia en la mano, el clic izquierdo dispara');
await pg.mouse.wheel(0,120);await U(()=>__umbral.step(0.02));
ok(await U(()=>__umbral.human.wsel)===3,'la rueda pasa a la siguiente arma');
await pg.keyboard.press('Digit1');await U(()=>__umbral.step(0.02));
// cinturón
await U(()=>{const h=__umbral.human;h.throws.bomba=2;h.throws.humo=1;h.potions=2;h.hp=50;h.maxhp=100});
await pg.keyboard.press('Digit7');await U(()=>__umbral.step(0.02));
ok(await U(()=>__umbral.human.bsel)==='bomba','la tecla 7 elige la bomba');
await pg.keyboard.press('KeyR');await U(()=>__umbral.step(0.02));
ok(await U(()=>__umbral.human.throws.bomba)===1,'R lanza el objeto elegido');
await pg.keyboard.press('Digit5');await pg.keyboard.press('KeyR');await U(()=>__umbral.step(0.02));
ok(await U(()=>__umbral.human.potions===1&&__umbral.human.hp>50),'con la poción elegida, R la toma');
await pg.screenshot({path:path.join(DIR,'1-barras.png')});
// inventario
await pg.keyboard.press('Tab');await pg.waitForTimeout(150);
ok(await U(()=>__umbral.invOpen&&!document.getElementById('invp').hidden),'TAB abre el inventario');
const t0=await U(()=>__umbral.matchT);await U(()=>{for(let i=0;i<20;i++)__umbral.step(0.05)});
// mover la katana de la mochila a la barra en lugar de la espada
await pg.click('#invp [data-z="bag"][title="Katana"]');await pg.waitForTimeout(80);
ok(await U(()=>!!document.querySelector('.icard .istats')),'al tocar un arma se ve su ficha con datos y comparación');
await pg.screenshot({path:path.join(DIR,'2-inventario.png')});
await pg.click('#invp [data-z="bar"][data-i="1"]');await pg.waitForTimeout(80);
st=await U(()=>{const h=__umbral.human;return{bar:h.bar.map(x=>x&&x.id),bag:h.bag.filter(Boolean).map(x=>x.id)}});
ok(st.bar[1]==='katana'&&st.bag.includes('espada'),'tocar otro lugar cambia las armas de lugar: '+st.bar.join());
// soltar
await pg.click('#invp [data-z="bag"][title="Espada"]');await pg.click('#invp [data-a="drop"]');
ok(await U(()=>!__umbral.human.bag.some(x=>x&&x.id==='espada')&&__umbral.drops.some(d=>d.def&&d.def.id==='espada')),'«Soltar» deja el arma en el piso');
await pg.keyboard.press('Tab');await pg.waitForTimeout(100);
ok(await U(()=>!__umbral.invOpen),'TAB lo cierra');
// al morir, suelta lo que llevaba
await U(()=>{const h=__umbral.human;h.hp=1;h.maxhp=100;h.iframe=0;const d=__umbral.drops.length;window.__d0=d});
await U(()=>{const U=__umbral;U.human.st.burn=5;for(let i=0;i<30&&U.human.alive;i++)U.step(0.05)});
ok(await U(()=>!__umbral.human.alive&&__umbral.drops.filter(d=>d.type==='item').length>window.__d0-1),'al morir suelta sus armas');
ok(!errs.length,'sin errores'+(errs.length?': '+[...new Set(errs)].slice(0,3).join(' | '):''));
await b.close();console.log(fails?'FALLAS: '+fails:'TODO OK');process.exit(fails?1:0);
