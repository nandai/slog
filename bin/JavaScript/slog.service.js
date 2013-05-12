(function(exports)
{
    // 値正常化
    function normalizeValue(value, max)
    {
        if (value >= max)
            value -= max;

        return value;
    }

    // ログバッファ
    var LogBuffer = function()
    {
        this.logBufferLineCount = 2000;                     // 循環ログバッファ行数
        this.logs = new Array(this.logBufferLineCount);     // 循環ログバッファ
        this.logHeadIndex = 0;                              // 循環ログバッファの先頭インデックス
        this.logLastIndex = 0;                              // 循環ログバッファの最終インデックス
        this.logLineCount = 0;                              // 出力したログの総行数
    };

    LogBuffer.prototype =
    {
        add: function(level, msg)
        {
            var m = this;

            // 循環ログバッファの最後尾にログ追加
            m.logs[m.logLastIndex] = {level: level, msg: msg};

            // 最終インデックス更新
            m.logLastIndex = normalizeValue(++m.logLastIndex, m.logs.length);
            m.logLineCount++;

            // 出力したログの総行数が循環ログバッファ行数を超えていたら、先頭インデックス更新（を開始する）
            if (m.logs.length < m.logLineCount)
                m.logHeadIndex = normalizeValue(++m.logHeadIndex, m.logs.length);
        },

        get: function(index)
        {
            index = normalizeValue(this.logHeadIndex + index, this.logs.length);
            return this.logs[index];
        },

        getBufferLineCount: function()
        {
            return this.logs.length;
        },

        // ログ行数取得
        // バッファ行数と総行数の小さい方を返す
        getLineCount: function(view)
        {
            return Math.min(this.logs.length, this.logLineCount);
        }
    };

    // シーケンスログサービス
    var SequenceLogService = function()
    {
        this.version = '1.2.3';
        this.ws = null;
        this.logFileListUpdateCallback = null;
        this.logViews = [];                     // シーケンスログビューの配列
        this.logBuffer = new LogBuffer();

        this.logBuffer.add('s', '--- Sequence Log Service Version ' + this.version + ' ---');
    };

    SequenceLogService.prototype =
    {
        // 初期化
        init: function(domain, logFileListUpdateCallback)
        {
            this.logFileListUpdateCallback = logFileListUpdateCallback;

            // WebSocket
            this.ws = new WebSocket('ws://' + domain + '/getLog');
            var self = this;

            this.ws.onmessage = function(e) {self.onMessage(e.data);};
        },

        // シーケンスログビューを追加する
        addLogView: function(logView)
        {
            this.logViews[this.logViews.length] = logView;

            if (logView.setLogBuffer)
                logView.setLogBuffer(this.logBuffer);
        },

        // WebSocket受信ハンドラ
        onMessage: function(msg)
        {
            var cmd = msg.substring(0, 4);

            // シーケンスログファイル一覧更新
            if (cmd === '0001')
            {
                var json = eval('(' + msg.slice(4) + ')');

                if (this.logFileListUpdateCallback)
                    this.logFileListUpdateCallback(json);
            }

            // ログ追加
            else if (cmd === '0002')
            {
                var level =   msg.charAt(4);
                var message = msg.slice(4 + 1);

                this.logBuffer.add(level, message);

                for (var i = 0; i < this.logViews.length; i++)
                    this.logViews[i].onUpdateLog(level, message);
            }
        },

        // シーケンスログを開く
        openLog: function(fileName)
        {
            var len = this.getStringBytes(fileName) + 1;
            var buffer = new ArrayBuffer(4 + 4 + len);
            var dataView = new DataView(buffer);

            dataView.setUint32(0, 1);       // コマンドNo
            dataView.setUint32(4, len);     // ファイル名の長さ
            this.setStringToDataView(dataView, 8, fileName);

            this.ws.send(dataView.buffer);
        },

        // Unicode文字列をUTF-8にした場合のバイト数を取得
        getStringBytes: function(str)
        {
            var len = str.length;
            var bytes = 0;

            for (var i = 0; i < len; i++)
            {
                var c = str.charCodeAt(i);

                if (c <= 0x7F)
                {
                    bytes += 1;
                }
                else if (c <= 0x07FF)
                {
                    bytes += 2;
                }
                else if (c <= 0xFFFF)
                {
                    bytes += 3;
                }
                else
                {
                    bytes += 4;
                }
            }

            return bytes;
        },

        // Unicode文字列をUTF-8でDataViewに設定
        setStringToDataView: function(dataView, offset, str)
        {
            var len = str.length;
            var pos = offset;

            for (var i = 0; i < len; i++)
            {
                var c = str.charCodeAt(i);

                if (c <= 0x7F)
                {
                    dataView.setUint8(pos, c);
                    pos += 1;
                }
                else if (c <= 0x07FF)
                {
                    dataView.setUint8(pos + 0, 0xC0 | (c >>> 6));
                    dataView.setUint8(pos + 1, 0x80 | (c & 0x3F));
                    pos += 2;
                }
                else if (c <= 0xFFFF)
                {
                    dataView.setUint8(pos + 0, 0xE0 |  (c >>> 12));
                    dataView.setUint8(pos + 1, 0x80 | ((c >>>  6) & 0x3F));
                    dataView.setUint8(pos + 2, 0x80 |  (c         & 0x3F));
                    pos += 3;
                }
                else
                {
                    dataView.setUint8(pos + 0, 0xF0 |  (c >>> 18));
                    dataView.setUint8(pos + 1, 0x80 | ((c >>> 12) & 0x3F));
                    dataView.setUint8(pos + 2, 0x80 | ((c >>>  6) & 0x3F));
                    dataView.setUint8(pos + 3, 0x80 |  (c         & 0x3F));
                    pos += 4;
                }
            }
        }
    };

    // 初期設定
    if (exports.slog === undefined)
        exports.slog = {};

    exports.slog.service = new SequenceLogService();
})(this);
