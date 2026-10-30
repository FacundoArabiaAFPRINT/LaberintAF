// Laberint AF: lo social y la rejugabilidad (pedido de Facu, octubre 2026, con su lista de lo que hace bueno a un juego): guía en la
// primera partida, contrato por partida, logros, color y accesorio, partida rápida, alianzas y traiciones con globitos
// de diálogo, gallinas y modo cabezón, momentos de la partida, resumen y «otra partida». Uso: node tests/t-social.mjs
import fs from 'node:fs';import path from 'node:path';import {need,ROOT,OUT} from '../tools/dep.mjs';const {chromium}=need('playwright');
const DIR=path.join(OUT,'umbral-social');fs.mkdirSync(DIR,{recursive:true});
let fails=0;const ok=(c,m)=>{console.log((c?'  ✓ ':'  ✗ ')+m);if(!c)fails++};
const b=await chromium.launch();const errs=[];const pg=await b.newPage({viewport:{width:1400,height:860}});
pg.on('pageerror',e=>errs.push(e.message));await pg.route(/fonts\.g/,r=>r.abort());
await pg.goto('file://'+path.join(ROOT,'index.html'));await pg.waitForTimeout(400);
const U=(f,a)=>pg.evaluate(f,a);
// menú: look, duración, logros a la vista
await pg.click('[data-lk="col"][data-v="3"]');await pg.click('[data-lk="acc"][data-v="sombrero"]');await pg.click('[data-dur="rapida"]');
const st=await U(()=>JSON.parse(localStorage.getItem('umbral_alijo')));
ok(st.look.col===3&&st.look.acc==='sombrero'&&st.dur==='rapida','se guardan el color, el accesorio y la partida rápida');
ok(await U(()=>!document.querySelector('[data-lk="acc"][data-v="corona"]').disabled),'todos los accesorios están disponibles (la corona también)');
ok(await U(()=>/LOGROS · 0\//.test(document.getElementById('card').textContent)),'el menú muestra los logros');
await pg.screenshot({path:path.join(DIR,'0-menu.png'),fullPage:true});
await pg.click('#startBtn');await pg.waitForTimeout(250);await U(()=>{const i=document.getElementById('intro');if(!i.hidden)i.click()});
ok(await U(()=>__umbral.matchLen===480&&__umbral.matchT<=480),'la partida rápida dura 8 minutos');
ok(await U(()=>__umbral.FIG.p0.P.acc==='sombrero'),'el personaje sale con su accesorio');
await U(()=>__umbral.step(.02));
ok(await U(()=>!!__umbral.human.job&&/CONTRATO/.test(document.getElementById('ctLine').textContent)),'hay un contrato a la vista: '+await U(()=>document.getElementById('ctLine').textContent));
await U(()=>{const U=__umbral,h=U.human,r=U.rooms.find(r=>r.type==='exit');h.x=r.x-30;h.y=r.y;h.hp=h.maxhp=99999;for(const x of U.hunters){x.alive=false;x.dorm=false};for(const m of U.mobs)m.alive=false;for(let i=0;i<30;i++)U.step(.05)});
ok(await U(()=>!document.getElementById('tip').hidden),'la primera partida tiene guía: '+await U(()=>document.getElementById('tip').textContent));
// contrato: se cumple y paga
const ct=await U(()=>{const U=__umbral,h=U.human;h.job=Object.assign({v:0,done:false},U.CONTRACTS.find(c=>c.k==='cofres'));const g0=h.gold;for(let i=0;i<3;i++){const c={x:h.x,y:h.y,open:false,rich:false,claim:-1};U.chests.push(c)}
  for(const c of U.chests.slice(-3)){c.x=h.x;c.y=h.y;U.drops.length=0;for(const q of U.players)if(q!==h)q.x=h.x+400;U.step(.02);window.dispatchEvent(new KeyboardEvent('keydown',{code:'KeyE'}))}return{done:h.job.done,v:h.job.v,g:h.gold-g0}});
ok(ct.done&&ct.g>=50,'abrir 3 cofres cumple el contrato y paga el oro ('+ct.v+'/3)');
// logro (solo medalla) y aviso
const g0=await U(()=>JSON.parse(localStorage.getItem('umbral_alijo')).gold);await U(()=>__umbral.ach('buena'));
const ac=await U(()=>({a:JSON.parse(localStorage.getItem('umbral_alijo')),t:!document.getElementById('toast').hidden}));
ok(ac.a.ach.includes('buena')&&ac.a.gold===undefined&&g0===undefined&&ac.t,'un logro queda como medalla, sin oro, y muestra el aviso');
// alianza y traición
const al=await U(()=>{const U=__umbral,h=U.human,b=U.players.find(p=>p!==h&&p.alive);b.x=h.x+18;b.y=h.y;b.coop=true;b.hostile.clear();h.keys.delete(b.id);for(let i=0;i<30;i++){if(U.allyAsk(b),b.ally)break;b.allyCD=0}window.__b=b;return{ok:!!b.ally,bub:!!b.bubble}});
ok(al.ok&&al.bub,'se puede aliar con otro jugador, que contesta con un globito');
await U(()=>{for(let i=0;i<4;i++)__umbral.step(.05)});await pg.screenshot({path:path.join(DIR,'1-alianza.png'),clip:{x:450,y:230,width:500,height:400}});
const tr=await U(()=>{const U=__umbral,h=U.human,b=window.__b;U.betray(b);return{fight:b.fight===h,host:(b.hostile.get(0)||0)>U.time,ach:JSON.parse(localStorage.getItem('umbral_alijo')).ach.includes('traicionado')}});
ok(tr.fight&&tr.host&&tr.ach,'un aliado puede traicionarte (y ganás el logro «Ingenuo»)');
// eventos absurdos
const ev=await U(()=>{const U=__umbral;const n0=U.mobs.filter(m=>m.type==='gallina').length;U.EVENTS.gallinas.go();U.EVENTS.cabezon.go();for(let i=0;i<10;i++)U.step(.05);return U.mobs.filter(m=>m.type==='gallina'&&m.alive).length-n0});
ok(ev>=10,'el evento ¡GALLINAS! llena el laberinto de gallinas ('+ev+')');
await pg.screenshot({path:path.join(DIR,'2-gallinas.png')});
// fin: momentos, resumen y otra partida
await U(()=>{const U=__umbral;U.extract(U.human);for(let i=0;i<60;i++)U.step(.05)});await pg.waitForTimeout(200);
ok(await U(()=>__umbral.state==='end'&&!!document.querySelector('.end-moments')&&!!document.getElementById('rematchBtn')),'al terminar se ven los momentos de la partida y «OTRA PARTIDA»');
ok(await U(()=>JSON.parse(localStorage.getItem('umbral_alijo')).ach.includes('escape')),'escapar da el logro «Primera salida»');
await pg.screenshot({path:path.join(DIR,'3-fin.png'),fullPage:true});
await pg.click('#copyBtn');await pg.waitForTimeout(150);ok(await U(()=>/COPIADO/.test(document.getElementById('copyBtn').textContent)),'«Copiar resumen» deja el resumen listo para pegar');
await pg.click('#rematchBtn');await pg.waitForTimeout(300);
ok(await U(()=>__umbral.state==='play'),'«OTRA PARTIDA» arranca una nueva al toque');
ok(await U(()=>!__umbral.human.tut),'la guía ya no aparece en la segunda partida');
ok(!errs.length,'sin errores'+(errs.length?': '+[...new Set(errs)].slice(0,3).join(' | '):''));
await b.close();console.log(fails?'FALLAS: '+fails:'TODO OK');process.exit(fails?1:0);
