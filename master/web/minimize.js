import { minify } from 'html-minifier';
import fs from 'fs';
import zlib from 'zlib';

try {
    console.log('Minimizing ./index.html');
    const data = fs.readFileSync('./index.html', 'utf8');
    let result = minify(data, {
        removeAttributeQuotes: true,
        collapseWhitespace: true,
        removeComments: true,
        removeRedundantAttributes: true,
        removeScriptTypeAttributes: true,
        removeStyleLinkTypeAttributes: true,
        removeTagWhitespace: true,
        useShortDoctype: true,
        minifyCSS: true,
        minifyJS: true
    });
    
    // Gzip compress the minified HTML
    console.log('Compressing with gzip');
    const compressed = zlib.gzipSync(result);
    const byteArray = Array.from(compressed);

    // turn into c array string
    let byteArrayStr = '';
    for (let i = 0; i < byteArray.length; i++) {
        if (i % 16 === 0) {
            byteArrayStr += '\n  ';
        }
        byteArrayStr += '0x' + byteArray[i].toString(16).padStart(2, '0');
        if (i < byteArray.length - 1) {
            byteArrayStr += ', ';
        }
    }

    const page_h = '#ifndef WEB_PAGE_H\n' +
        '#define WEB_PAGE_H\n' +
        '#include <pgmspace.h>\n' +
        '\n' +
        'const uint8_t WEB_PAGE_HTML[' + byteArray.length + '] PROGMEM = {' + byteArrayStr + '\n};\n' +
        '\n' +
        '#endif\n';

    fs.writeFileSync('../include/web_page.h', page_h);
    console.log('Generated ../include/web_page.h');
} catch (err) {
    console.error(err);
}
