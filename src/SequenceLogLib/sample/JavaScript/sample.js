var fs =   require('fs');
var http = require('http');
var slog = require('../../../../bin/Node.js/slog');

slog.setConfig('ws://127.0.0.1:8080', 'sample.js.log', 'ALL', 'slog', 'gols');
var log = slog.stepIn('sample.js', 'main');

var server = http.createServer(function(req, res)
{
    var log = slog.stepIn('sample.js', 'createServer');
    log.d(req.method + ' ' + req.url);
    res.writeHead(200, {'Content-Type': 'text/html'});
    res.end(fs.readFileSync('www/index.html'));
    log.stepOut();
})
.listen(1234);

var io = require('socket.io').listen(server);
io.sockets.on('connection', function(socket)
{
    var log = slog.stepIn('sockets', 'connection');

    var address = socket.handshake.address;
    log.d(address.address + ':' + address.port + 'から接続されました。');

    socket.on('message', function(data)
    {
        var log = slog.stepIn('socket', 'message');

        if (data.length === 0) {
            log.w('クライアントから空文字列を受信しました。');
        } else {
            log.d('クライアントから「' + data + '」を受信しました。');
        }

        sockets_emit('message', '受信した文字列は「' + data + '」、文字数は' + data.length + 'です。');
        log.stepOut();
    });
    log.stepOut();
});

function sockets_emit(event, data) {
    var log = slog.stepIn('sockets', 'emit');
    log.d('クライアントに「' + data + '」を送信します。');
    io.sockets.emit(event, data);
    log.stepOut();
}

log.stepOut();
