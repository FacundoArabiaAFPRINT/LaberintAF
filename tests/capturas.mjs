// Capturas de Umbral con el azar fijo (mismo laberinto en cada corrida), para comparar el estilo gráfico antes y después.
// Toma: la escena de arranque, una «galería» con jugadores, cazadores y bestias alrededor del jugador, y un recorte ampliado.
// Mide también cuánto tarda en dibujar un cuadro. Uso: node tests/capturas.mjs [carpeta de salida] [semilla]
import fs from 'node:fs';import path from 'node:path';import {need,ROOT,OUT} from '../tools/dep.mjs';const {chromium}=need('playwright');
const DIR=path.resolve(process.argv[2]||path.join(OUT,'umbral-estilo')),SEED=+process.argv[3]||7;fs.mkdirSync(DIR,{recursive:true});
const b=await chromium.launch();const pg=await b.newPage({viewport:{width:1400,height:860}});const errs=[];
pg.on('pageerror',e=>errs.push(e.message));await pg.route(/fonts\.g/,r=>r.abort());
await pg.addInitScript(s=>{let a=s>>>0;Math.random=()=>{a=a+0x6D2B79F5|0;let t=Math.imul(a^a>>>15,1|a);t=t+Math.imul(t^t>>>7,61|t)^t;return((t^t>>>14)>>>0)/4294967296}},SEED);
await pg.goto('file://'+path.join(ROOT,'index.html'));await pg.waitForTimeout(500);
await pg.screenshot({path:path.join(DIR,'0-menu.png')});
await pg.click('#startBtn');await pg.waitForTimeout(300);
await pg.evaluate(()=>{const i=document.getElementById('intro');if(!i.hidden)i.click()});
await pg.evaluate(()=>{for(let i=0;i<60;i++)__umbral.step(0.05)});await pg.waitForTimeout(100);
await pg.screenshot({path:path.join(DIR,'1-escena.png')});
// Galería (en la sala más grande): bestias, cazadores y otros jugadores en ronda alrededor del jugador, quietos
await pg.evaluate(()=>{const U=__umbral,h=U.human,seen=new Set(),pick=[];
 const r=U.rooms.filter(r=>r.type==='hall'||r.type==='boss').sort((a,b)=>(b.x1-b.x0)*(b.y1-b.y0)-(a.x1-a.x0)*(a.y1-a.y0))[0];if(r){h.x=r.x;h.y=r.y}
 for(const e of [...U.hunters,...U.mobs,...U.players.filter(p=>p!==h)]){const k=e.ct||e.type;if(!e.alive||seen.has(k))continue;seen.add(k);pick.push(e)}
 const n=Math.min(pick.length,14);h.hp=h.maxhp=9999;
 for(let i=0;i<n;i++){const e=pick[i],a=i/n*Math.PI*2,r=i%2?52:34;e.x=h.x+Math.cos(a)*r;e.y=h.y+Math.sin(a)*r;e.aim=a+Math.PI;e.vis=true}
 window.__gal=pick.slice(0,n);U.step(0.0001)});
await pg.evaluate(()=>{const i=document.getElementById('intro');if(!i.hidden)i.click();__umbral.step(0.0001)});
await pg.screenshot({path:path.join(DIR,'2-galeria.png')});
await pg.screenshot({path:path.join(DIR,'3-recorte.png'),clip:{x:450,y:230,width:500,height:400}});
// Golpe de historieta: un enemigo pegado al jugador y un ataque
await pg.evaluate(()=>{const U=__umbral,h=U.human,e=window.__gal&&window.__gal.find(x=>x.kind==='mob')||window.__gal[0];if(!e)return;e.x=h.x+11;e.y=h.y;h.aim=0;e.hp=e.maxhp=9999;window.__tg=e});
await pg.keyboard.down('KeyJ');await pg.evaluate(()=>{for(let i=0;i<3;i++)__umbral.step(0.03)});await pg.keyboard.up('KeyJ');
await pg.evaluate(()=>{for(let i=0;i<4;i++){__umbral.human.aim=0;__umbral.step(0.025)}});
await pg.screenshot({path:path.join(DIR,'5-golpe.png'),clip:{x:500,y:250,width:400,height:300}});
// Retratos ampliados de cada figura (si la página tiene __umbral.portrait)
if(await pg.evaluate(()=>!!__umbral.portrait)){
 await pg.evaluate(()=>{const keys=Object.keys(__umbral.FIG).filter(k=>!/^p[1-9]$/.test(k)),cv=document.createElement('canvas'),cols=6,cw=230,chh=230;
  cv.width=cols*cw;cv.height=Math.ceil(keys.length/cols)*chh;const g=cv.getContext('2d');g.fillStyle='#8a7a5a';g.fillRect(0,0,cv.width,cv.height);
  keys.forEach((k,i)=>{const big={gigante:1,diablo:1,jefe:1}[k]?4.2:{vibora:1,arana:1}[k]?6:8;__umbral.portrait(g,k,(i%cols)*cw+cw/2,((i/cols)|0)*chh+chh/2,big,-Math.PI/2);g.fillStyle='#000';g.font='14px sans-serif';g.fillText(k,(i%cols)*cw+8,((i/cols)|0)*chh+18)});
  const im=new Image();im.src=cv.toDataURL();im.id='retratos';im.style.cssText='position:fixed;left:0;top:0;z-index:99;background:#000';document.body.appendChild(im)});
 await pg.waitForTimeout(200);const bb=await pg.locator('#retratos').boundingBox();await pg.screenshot({path:path.join(DIR,'4-retratos.png'),clip:bb});await pg.evaluate(()=>document.getElementById('retratos').remove())}
const ms=await pg.evaluate(()=>{const t0=performance.now();for(let i=0;i<60;i++)__umbral.step(0.0001);return(performance.now()-t0)/60});
console.log('Cuadro: '+ms.toFixed(1)+' ms (actualizar + dibujar)');
console.log(errs.length?'ERRORES: '+errs.join(' | '):'Sin errores');console.log('Capturas en '+DIR);await b.close();
