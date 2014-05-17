(function(exports)
{
    'use strict';

    // シーケンスログサービス
    var SequenceLogService = function()
    {
        this.version = '1.2.8';
        this.ws = null;
        this.logFileListUpdateCallback = null;
        this.logViews = [];                     // シーケンスログビューの配列
        this.logBuffer = null;
    };

    SequenceLogService.prototype =
    {
        // 初期化
        init: function(domain, buffer, logFileListUpdateCallback)
        {
            if (buffer === null || buffer === undefined)
                buffer = exports.slog.utils.emptyBuffer;

            this.logBuffer = buffer;
            this.logBuffer.add(log('s', '--- Sequence Log Service Version ' + this.version + ' ---'));
            this.logFileListUpdateCallback = logFileListUpdateCallback;

            // WebSocket
            var protocol = (('http:' === document.location.protocol) ? 'ws://' : 'wss://');
            this.ws = new WebSocket(protocol + domain + '/getLog');
            var self = this;

            this.ws.onmessage = function(e) {onMessage(self, e.data);};
        },

        // シーケンスログビューを追加する
        addLogView: function(logView)
        {
            this.logViews[this.logViews.length] = logView;

            if (logView.setBuffer)
                logView.setBuffer(this.logBuffer);
        },

        // シーケンスログをSequenceLog.exeで開く
        openLog: function(fileName)
        {
            var len = exports.slog.utils.getStringBytes(fileName) + 1;
            var buffer = new ArrayBuffer(4 + 4 + len);
            var dataView = new DataView(buffer);

            dataView.setUint32(0, 1);       // コマンドNo
            dataView.setUint32(4, len);     // ファイル名の長さ
            exports.slog.utils.setStringToDataView(dataView, 8, fileName);

            this.ws.send(dataView.buffer);
        }
    };

    // ログデータ
    function log(level, msg)
    {
        return {flag: level, msg: msg};
    }

    // WebSocket受信ハンドラ
    function onMessage(service, msg)
    {
        var cmd = msg.substring(0, 4);

        // シーケンスログファイル一覧更新
        if (cmd === '0001')
        {
            var json = eval('(' + msg.slice(4) + ')');

            if (service.logFileListUpdateCallback)
                service.logFileListUpdateCallback(json);
        }

        // ログ追加
        else if (cmd === '0002')
        {
            var level =   msg.charAt(4);
            var message = msg.slice(4 + 1);

            var removeCount = service.logBuffer.add(log(level, message));

            for (var i = 0; i < service.logViews.length; i++)
                service.logViews[i].onUpdateBuffer(level, message, removeCount);
        }
    }

    // 初期設定
    if (exports.slog === undefined)
        exports.slog = {};

    exports.slog.service = new SequenceLogService();

    setInterval(function()
    {
        for (var i = 0; i < exports.slog.service.logViews.length; i++)
            exports.slog.service.logViews[i].draw();
    },
    50);
})(this);
