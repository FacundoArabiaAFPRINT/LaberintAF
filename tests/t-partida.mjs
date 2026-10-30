// Laberint AF (juego de Facu, index.html): arranca sin errores en compu y celular, se juega un rato
// moviéndose y atacando, la partida avanza (reloj, bestias, cazadores) y termina en una pantalla de fin.
// Capturas en .salidas/umbral/. Uso: node tests/t-partida.mjs [segundos de partida simulados, 120 por defecto]
import fs from 'node:fs';import path from 'node:path';import {need,ROOT,OUT} from '../tools/dep.mjs';const {chromium}=need('playwright');
const SEG=+process.argv[2]||120,DIR=path.join(OUT,'umbral');fs.mkdirSync(DIR,{recursive:true});
let fails=0;const ok=(c,m)=>{console.log((c?'  ✓ ':'  ✗ ')+m);if(!c)fails++};
const b=await chromium.launch();
for(const [nom,vp] of [['compu',{width:1400,height:860}],['celular',{width:844,height:390}]]){
 const errs=[];const pg=await b.newPage({viewport:vp,isMobile:nom==='celular',hasTouch:nom==='celular'});
 pg.on('pageerror',e=>errs.push(e.message));pg.on('console',m=>{if(m.type()==='error'&&!/fonts|ERR_FAILED/.test(m.text()))errs.push(m.text())});
 await pg.route(/fonts\.g/,r=>r.abort());
 await pg.goto('file://'+path.join(ROOT,'index.html'));await pg.waitForTimeout(800);
 ok(await pg.evaluate(()=>window.__umbral?.state==='menu'&&!!document.querySelector('#startBtn')),nom+': menú con «Entrar al laberinto»');
 await pg.screenshot({path:path.join(DIR,nom+'-menu.png')});
 await pg.click('#startBtn');await pg.waitForTimeout(600);
 const s0=await pg.evaluate(()=>({st:__umbral.state,t:__umbral.matchT,n:__umbral.players.length,h:__umbral.hunters.length}));
 ok(s0.st==='play'&&s0.n>1&&s0.h>0,nom+': partida en juego ('+s0.n+' jugadores, '+s0.h+' cazadores)');
 await pg.screenshot({path:path.join(DIR,nom+'-intro.png')});
 // Se mueve (WASD), corre y ataca mientras se simula la partida de a pasos fijos
 const keys=['w','d','s','a'];
 for(let i=0;i<SEG/2;i++){
  const k=keys[i%4];await pg.keyboard.down(k);if(i%3===0)await pg.keyboard.down('Shift');
  await pg.evaluate(()=>{for(let j=0;j<40;j++){if(__umbral.state!=='play')break;__umbral.step(0.05)}});
  if(nom==='compu'){await pg.mouse.move(700+200*Math.cos(i),430+150*Math.sin(i));await pg.mouse.down();await pg.mouse.up()}
  await pg.keyboard.up(k);await pg.keyboard.up('Shift');
  if(i===8)await pg.screenshot({path:path.join(DIR,nom+'-juego.png')});
  if(await pg.evaluate(()=>__umbral.state!=='play'))break}
 const s1=await pg.evaluate(()=>({st:__umbral.state,t:__umbral.matchT,mobs:__umbral.mobs.length,feed:__umbral.feedLog.length,hp:__umbral.human.hp,alive:__umbral.human.alive}));
 ok(s1.t<s0.t||s1.st!=='play',nom+': el reloj corre ('+Math.round(s0.t)+' → '+Math.round(s1.t)+' s, estado '+s1.st+', '+s1.mobs+' bestias, crónica '+s1.feed+')');
 // Termina la partida: se agota el tiempo
 await pg.evaluate(()=>{let n=0;while(__umbral.state==='play'&&n++<40000)__umbral.step(0.05)});await pg.waitForTimeout(300);
 const fin=await pg.evaluate(()=>({st:__umbral.state,h1:document.querySelector('#card h1')?.textContent,vis:!document.querySelector('#screen').hidden}));
 ok(fin.st==='end'&&fin.vis&&!!fin.h1,nom+': pantalla de fin «'+fin.h1+'»');
 await pg.screenshot({path:path.join(DIR,nom+'-fin.png')});
 await pg.click('#againBtn');await pg.waitForTimeout(300);
 ok(await pg.evaluate(()=>__umbral.state==='menu'),nom+': «Volver al menú» vuelve al menú');
 ok(!errs.length,nom+': sin errores'+(errs.length?': '+[...new Set(errs)].slice(0,3).join(' | '):''));
 await pg.close()}
await b.close();console.log('Capturas en '+DIR);console.log(fails?'FALLAS: '+fails:'TODO OK');process.exit(fails?1:0);
