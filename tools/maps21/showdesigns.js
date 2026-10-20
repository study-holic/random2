const a=require('../../data/designs-0.0.3.json'),b=require('../../data/designs-1.0.0.json');
const N=['Ryth','Vesper','Vendetta','Cassi','Issac','Stellar','Prism','Marcella','Willow','Ren','Varg','Tavi','HeartOfRuin'];
const s=x=>x?(x.kind+' cd'+x.cd+' ch'+x.charges+' r'+x.range+(x.radius!==undefined?' rad'+x.radius:'')+' m'+x.mod+(x.dur!==undefined?' d'+x.dur:'')+' tg'+x.targets):'-';
for(const t in b.champions){const c=b.champions[t];
console.log(t,N[t],'pas'+c.passive,'atk'+c.attack_type,'hp',c.hp,'+'+c.hp_g,'str',c.str,'+'+c.str_g,'foc',c.foc,'+'+c.foc_g,'mv',c.move,'rng',c.range,'vis',c.vis,'cm',c.campmult);console.log('   A1',s(c.a1));console.log('   A2',s(c.a2));
const o=a.champions[t];if(o&&JSON.stringify(o)!==JSON.stringify(c))console.log('   CHANGED:',Object.keys(c).filter(k=>JSON.stringify(o[k])!==JSON.stringify(c[k])).map(k=>k+' '+JSON.stringify(o[k])+' -> '+JSON.stringify(c[k])).join('; '))}
console.log('CAMPS');for(const t in b.camps){const c=b.camps[t];console.log(t,JSON.stringify(c));const o=a.camps[t];console.log('  old',o?JSON.stringify(o)===JSON.stringify(c)?'same':JSON.stringify(o):'n/a')}
