// Exporta los datos del juego (armas, cazadores, bestias, jefes, tienda, contratos, logros, personalización) a JSON y CSV
// para el proyecto de Unreal Engine 5 (unreal/LaberintAF/Content/Data). Así el balance de la versión web pasa tal cual.
// Uso: node tools/exportar-datos.mjs
import fs from 'node:fs';import path from 'node:path';import {need,ROOT} from './dep.mjs';const {chromium}=need('playwright');
const OUTD=path.join(ROOT,'unreal/LaberintAF/Content/Data');fs.mkdirSync(OUTD,{recursive:true});
const b=await chromium.launch();const pg=await b.newPage();await pg.route(/fonts\.g/,r=>r.abort());
// el juego está encerrado en una función: en una copia temporal se suma una línea que deja ver sus datos
const html=fs.readFileSync(path.join(ROOT,'index.html'),'utf8');if(!html.includes('window.__umbral={'))throw new Error('no encuentro window.__umbral en index.html');
const tmp=path.join(ROOT,'.salidas','exportar.html');fs.mkdirSync(path.dirname(tmp),{recursive:true});
fs.writeFileSync(tmp,html.replace('window.__umbral={','window.__datos={IT,LOOT,RAR,HDEF,spawnHunters,MOBDEF,SHOP,CONTRACTS,ACH,CHARS,COLORS,shade,SKINC,HAIRC,HSTY,BARBAS,ACCS,CAPAS,EVENTS,FARM,FOG_ST,EV_T,CW,CH,CS,TILE};window.__umbral={'));
await pg.goto('file://'+tmp);await pg.waitForTimeout(400);
const D=await pg.evaluate(()=>{const {IT,LOOT,RAR,HDEF,spawnHunters,MOBDEF,SHOP,CONTRACTS,ACH,CHARS,COLORS,shade,SKINC,HAIRC,HSTY,BARBAS,ACCS,CAPAS,EVENTS,FARM,FOG_ST,EV_T,CW,CH,CS,TILE}=window.__datos;
  const items=Object.entries(IT).map(([id,v])=>({Name:id,Nombre:v.n||id,Ranura:v.slot||'',Danio:v.dmg||0,Alcance:v.range||0,Barrido:v.arc||0,Enfriamiento:v.cd||0,Nivel:v.tier||0,Aturde:v.stun||0,ProbAturdir:v.stunCh||0,Empuje:v.kb||0,Velocidad:v.spd||0,Cargador:v.mag||0,Luz:v.r||0,Armadura:v.armor||0,Genero:v.g||''}));
  const loot=LOOT.map(([id,w])=>({Name:id,Peso:w}));
  const rar=RAR.map((r,i)=>({Name:['comun','raro','epico','legendario'][i],Multiplicador:r.m,Color:r.col||'#ffffff'}));
  const hunters=[];for(let L=1;L<=10;L++){const D=HDEF[L],p={x:0,y:0,id:99};const hs=spawnHunters(p,L,{x:0,y:0});const k=hs.find(h=>h.keyHolder)||hs[0];
    hunters.push({Name:D.type,Nivel:L,Nombre:D.name,Corto:D.short,Aviso:D.call,Descripcion:D.desc,Cantidad:hs.length,Vida:k.maxhp,Danio:k.dmg,Velocidad:Math.round(k.speed*100)/100,Radio:k.r,Escala:k.scale||1,Enfriamiento:k.cdMax,Mascota:k.pet||''})}
  const mobs=Object.entries(MOBDEF).map(([id,m])=>({Name:id,Nombre:m.name,Vida:m.hp,Danio:m.dmg,Velocidad:Math.round(m.speed*1.3*100)/100,Alerta:m.aggro,Oro:m.gold,Radio:m.r,Jefe:!!m.boss,Fantasma:!!m.ghost}));
  const shop=SHOP.map((s,i)=>({Name:'s'+i,Nombre:s.n,Descripcion:s.d,Precio:s.c}));
  const contracts=CONTRACTS.map(c=>({Name:c.k,Texto:c.n,Meta:c.goal,Oro:c.r}));
  const ach=ACH.map(a=>({Name:a.k,Nombre:a.n,Descripcion:a.d}));
  const chars=CHARS.map(c=>({Name:c.k,Nombre:c.n,Descripcion:c.d}));
  const look={abrigos:COLORS.map(c=>shade(c,1.55)),pieles:SKINC,pelos:HAIRC,peinados:HSTY,barbas:BARBAS,accesorios:ACCS,capas:CAPAS};
  const ev=Object.keys(EVENTS);
  const reglas={duracion:900,duracionRapida:480,farmeo:FARM,jugadores:10,etapasNiebla:FOG_ST,eventos:EV_T,eventosNombres:ev,mapa:{celdas:[CW,CH],tamCelda:CS,tile:TILE},velocidades:{caminar:58,correr:88,rodar:165,factorBots:1.3},bestias:66,cofresCallejon:40,armasTiradas:42,trampas:{pinches:22,barriles:22},escotillaSegundos:10};
  return{items,loot,rar,hunters,mobs,shop,contracts,ach,chars,look,reglas}});
await b.close();
// formato de DataTable de Unreal: primera columna = nombre de la fila
const tbl={Armas:D.items,Botin:D.loot,Rarezas:D.rar,Cazadores:D.hunters,Bestias:D.mobs,Tienda:D.shop,Contratos:D.contracts,Logros:D.ach,HabilidadesPersonaje:D.chars};
for(const[k,v]of Object.entries(tbl)){const keys=Object.keys(v[0]);fs.writeFileSync(path.join(OUTD,k+'.csv'),[ '---,'+keys.slice(1).join(','),...v.map(r=>keys.map(x=>{const s=String(r[x]);return/[",\n]/.test(s)?'"'+s.replace(/"/g,'""')+'"':s}).join(','))].join('\n')+'\n')}
fs.writeFileSync(path.join(OUTD,'Personalizacion.json'),JSON.stringify(D.look,null,1));
fs.writeFileSync(path.join(OUTD,'Reglas.json'),JSON.stringify(D.reglas,null,1));
console.log('Exportado:',Object.entries(tbl).map(([k,v])=>k+' ('+v.length+')').join(', '),'+ Personalizacion.json, Reglas.json');
