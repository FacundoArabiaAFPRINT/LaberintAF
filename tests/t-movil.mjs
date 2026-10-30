// Laberint AF: movimiento y acción (pedido de Facu, octubre 2026: «lo siento lento y con falta de acción»). Caminar y correr más
// rápido, bestias que patrullan, y en el celular: correr con el dedo a fondo y atacar solo al apuntar a un enemigo a tiro.
// Uso: node tests/t-movil.mjs
import path from 'node:path';import {need,ROOT} from '../tools/dep.mjs';const {chromium}=need('playwright');
let fails=0;const ok=(c,m)=>{console.log((c?'  ✓ ':'  ✗ ')+m);if(!c)fails++};
const b=await chromium.launch();const errs=[];
const open=async(o)=>{const pg=await b.newPage(o);pg.on('pageerror',e=>errs.push(e.message));await pg.route(/fonts\.g/,r=>r.abort());await pg.goto('file://'+path.join(ROOT,'index.html'));await pg.waitForTimeout(400);
  await pg.click('#startBtn');await pg.waitForTimeout(200);await pg.evaluate(()=>{const i=document.getElementById('intro');if(!i.hidden)i.click();const U=__umbral,h=U.human,r=U.rooms.find(r=>r.type==='exit');h.x=r.x-40;h.y=r.y;h.hp=h.maxhp=99999;for(const x of U.hunters){x.alive=false;x.dorm=false};for(const m of U.mobs)if(Math.hypot(m.x-h.x,m.y-h.y)<200)m.alive=false});return pg};
let pg=await open({viewport:{width:1400,height:860}});
const spd=async(keys)=>{for(const k of keys)await pg.keyboard.down(k);const v=await pg.evaluate(()=>{let m=0;for(let i=0;i<6;i++){__umbral.step(.03);m=Math.max(m,__umbral.human.spd)}return m});for(const k of keys)await pg.keyboard.up(k);await pg.evaluate(()=>__umbral.step(.05));return v};
const w=await spd(['KeyD']),r=await spd(['KeyA','ShiftLeft']);
ok(w>50&&w<62,'camina a ~58 (antes 40): '+w.toFixed(1));ok(r>78&&r<95,'corre a ~88 (antes 62): '+r.toFixed(1));
const pat=await pg.evaluate(()=>{const U=__umbral,ms=U.mobs.filter(m=>m.alive&&m.type!=='jefe'&&!m.ghost).slice(0,20),p0=ms.map(m=>[m.x,m.y]);for(let i=0;i<120;i++)U.step(.05);return ms.filter((m,i)=>Math.hypot(m.x-p0[i][0],m.y-p0[i][1])>25).length});
ok(pat>=8,'las bestias patrullan (se movieron '+pat+' de 20 en 6 s)');
await pg.close();
pg=await open({viewport:{width:844,height:390},isMobile:true,hasTouch:true});
const cv=await pg.evaluate(()=>{const c=document.getElementById('c'),r=c.getBoundingClientRect();return{h:c.height,css:r.height}});
ok(cv.h<=cv.css*1.16,`en el celular la imagen se dibuja más liviana (${cv.h} px para ${Math.round(cv.css)} px de pantalla)`);
const au=await pg.evaluate(()=>{const U=__umbral,h=U.human,z=U.mkMob('zombi',{x:h.x+12,y:h.y});z.hp=z.maxhp=999;U.mobs.push(z);h.aim=0;h.wsel=0;
  U.touch.aim={id:9,ox:200,oy:100,x:216,y:100};const hp0=z.hp;for(let i=0;i<20;i++){z.x=h.x+12;z.y=h.y;U.step(.05)}const hit=z.hp<hp0;
  z.x=h.x-12;const hp1=z.hp;for(let i=0;i<20;i++){z.x=h.x-12;z.y=h.y;U.step(.05)}U.touch.aim=null;return{hit,miss:z.hp===hp1}});
ok(au.hit,'con el dedo derecho apuntando a un enemigo a tiro, ataca solo');
ok(au.miss,'si el enemigo está a la espalda, no ataca al aire');
const run=await pg.evaluate(()=>{const U=__umbral,h=U.human;U.touch.move={id:3,ox:100,oy:200,x:116,y:200};let m=0;for(let i=0;i<6;i++){U.step(.03);m=Math.max(m,h.spd)}U.touch.move=null;return m});
ok(run>78,'con el dedo izquierdo a fondo corre: '+run.toFixed(1));
ok(!errs.length,'sin errores'+(errs.length?': '+[...new Set(errs)].slice(0,3).join(' | '):''));
await b.close();console.log(fails?'FALLAS: '+fails:'TODO OK');process.exit(fails?1:0);
