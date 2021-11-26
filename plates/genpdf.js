const PDFGenerator = require('pdfkit')
const fs = require('fs')
const SVGtoPDF = require('svg-to-pdfkit');
// instantiate the library
let doc = new PDFGenerator({size:"A4"})

PDFGenerator.prototype.addSVG = function(svg, x, y, options) {
    return SVGtoPDF(this, svg, x, y, options), this;
  };

doc.save();


const sx=0;
const sy=0;
const dx=72*(20*0.393701);
const dy=dx;

for(i=0;i<4000;i++){
    const xp=Math.random()*dx-dx/2;
    const yp=Math.random()*dy-dy/2;
    doc.rect(doc.page.width/2+xp,doc.page.height/2+yp, 10, 10).stroke();

}





  doc.circle(doc.page.width/2, doc.page.height/2, 72*(20*0.393701) /2   )
  .dash(5, {space: 10})
  .stroke();
doc.restore();


const cross=15
doc.moveTo(doc.page.width/2-cross, doc.page.height/2) 
   .lineTo(doc.page.width/2+cross, doc.page.height/2)
//   .dash(5, {space: 2})
   .stroke();

doc.moveTo(doc.page.width/2, doc.page.height/2-cross) 
   .lineTo(doc.page.width/2, doc.page.height/2+cross)
//   .dash(5, {space: 2})
   .stroke();

const p= 72*(11.5*0.393701)/2;
const s=72;
const s2=s/2;




doc.image('CCTagOld/markersToPrint/generators/0000.png', doc.page.width/2-p-s2, doc.page.height/2-p-s2, {fit: [s, s], align: 'center', valign: 'center'})
doc.image('CCTagOld/markersToPrint/generators/0001.png', doc.page.width/2+p-s2, doc.page.height/2-p-s2, {fit: [s, s], align: 'center', valign: 'center'})
doc.image('CCTagOld/markersToPrint/generators/0002.png', doc.page.width/2+p-s2, doc.page.height/2+p-s2, {fit: [s, s], align: 'center', valign: 'center'})
doc.image('CCTagOld/markersToPrint/generators/0003.png', doc.page.width/2-p-s2, doc.page.height/2+p-s2, {fit: [s, s], align: 'center', valign: 'center'})




doc.pipe(fs.createWriteStream('TurnTablePlate_white.pdf'))

doc.text('TurnTable Plate', { bold: true,
    align: 'center'
})

// write out file
doc.end()
