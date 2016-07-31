// require:
//     jquery.js
//     jquery.mousewheel.js
(function(exports)
{
    'use strict';

    if ($ == null)
        $ = jQuery;

    var SCROLLBOX_WIDTH = 16;           // スクロールボックスの幅
    var SCROLLBOX_MIN_HEIGHT = 12;      // スクロールボックスの最小の高さ

    var yMouse = 0;                     // ドラッグ開始時のy座標
    var orgLogVisibleTop = 0;           // ドラッグ開始時の可視行
    var scrollingView = null;           // スクロールバー操作中のビュー

    var focusView = null;               // フォーカスのあるビュー

    // テキストサイズ取得
    function getTextSize(text, font)
    {
        var span = $('<span style="font: ' + font + '">' + text + '</span>').appendTo(document.body);
        var width =  span.width();
        var height = span.height();

        span.remove();
        return {width: width, height: height};
    }

    // マウスダウン処理
    $(document).mousedown(function(event)
    {
        // 一旦フォーカスをnull設定
        focusView = null;
    });

    // マウスアップ処理
    $(document).mouseup(function()
    {
        scrollingView = null;
    });

   // Viewのメンバ変数定義
    var ViewMember = function()
    {
        this.canvas = null;
        this.ctx = null;
        this.font = '12pt Consolas';
        this.size = getTextSize('M', this.font);

        this.visibleTop = 0;            // 最初の可視行
        this.visibleLineCount = 20;     // 可視行数
        this.buffer = exports.slog.utils.emptyBuffer;
    };

    // View初期化
    function initView(view)
    {
        view.member = new ViewMember();
        var m = view.member;

        // canvasとコンテキストの設定
        m.canvas = view.get(0);
        m.ctx = m.canvas.getContext('2d');

        // マウスホイール処理
        view.mousewheel(function(event, delta, deltaX, deltaY)
        {
            var offset = (delta > 0 ? -3 : 3);
            scrollView(view, offset);
            return false;
        });

        // キー入力処理
        $(document).keydown(function(event)
        {
            return keydownView(view, event);
        });

        // マウスダウン処理（スクロールボックスのドラッグ開始）
        $(document).mousedown(function(event)
        {
            var target = $(event.target);
            var result = true;

            if (target.get(0).id === view.get(0).id)
            {
                var m = view.member;
                focusView = view;

                var boxRect = getScrollBoxRect(view);
//              var xOffset = event.offsetX;    FirefoxではoffsetXが未定義なため使えない
                var xOffset = event.pageX - view.offset().left;
                var yOffset = event.pageY - view.offset().top;

                if (boxRect.ptInRect(xOffset, yOffset))
                {
                    yMouse = event.clientY;
                    orgLogVisibleTop = m.visibleTop;
                    scrollingView = view;

                    result = false;     // ドラッグ中はテキスト等、選択されないようにする
                }
            }

            drawView(view);
            return result;
        });

        // マウス移動処理（スクロールボックスの移動）
        $(document).mousemove(function(event)
        {
            if (scrollingView !== view)
                return;

            var m = view.member;
            var boxRect = getScrollBoxRect(view);
            var lineCount = m.buffer.getCount();
            var linePixel = (m.canvas.height - boxRect.height()) / (lineCount - m.visibleLineCount);

            var offset = ~~((event.clientY - yMouse) / linePixel);

            m.visibleTop = orgLogVisibleTop + offset;
            normalizeVisibleTop(view);
            drawView(view);
        });
    }

    // ログ描画
    function drawView(view)
    {
        var m = view.member;

        if (m.ctx === null)
            return;

        var colors =
        {
            s: 'lightgreen',
            d: 'yellowgreen',
            i: 'white',
            w: 'yellow',
            e: 'red'
        };

        m.ctx.textBaseline = 'top';
        m.ctx.font = m.font;

        // canvasクリア
        m.ctx.clearRect(0, 0, m.canvas.width, m.canvas.height);

        // 座標
        var x = 0;
        var y = 0;

        // 整数化処理（動的サイズ変更等で浮動小数点が設定されても大丈夫なようにするため）
        m.visibleLineCount = ~~(m.visibleLineCount);
        normalizeVisibleTop(view);

        // 描画行数
        var visibleLineCount = getVisibleLineCount(view);

        // 行番号描画
        m.ctx.fillStyle = 'lightskyblue';

        for (var i = 0; i < visibleLineCount; i++)
        {
            var msg = (m.visibleTop + i + 1) + ': ';
            m.ctx.fillText(msg, x, y);

            y += m.size.height;
        }

        // テキスト描画
        x = m.size.width * 6;
        y = 0;

        for (var i = 0; i < visibleLineCount; i++)
        {
            var log = m.buffer.get(m.visibleTop + i);
            var msg = log.msg;

            m.ctx.fillStyle = colors[log.flag];
            m.ctx.fillText(msg, x, y);

            y += m.size.height;
        }

        // スクロールバー
        var boxRect = getScrollBoxRect(view);

        m.ctx.fillStyle = (focusView === view ? 'lightskyblue' : 'gray');
        m.ctx.fillRect(boxRect.left, boxRect.top, boxRect.width(), boxRect.height());
    }

    // 可視行数取得
    // 可視行数と総行数の小さい方を返す
    function getVisibleLineCount(view)
    {
        var m = view.member;
        return Math.min(m.visibleLineCount, m.buffer.getCount());
    }

    // スクロールボックスの矩形座標取得
    function getScrollBoxRect(view)
    {
        var m = view.member;

        var visibleLineCount = getVisibleLineCount(view);
        var lineCount = m.buffer.getCount();
        var boxHeight = visibleLineCount / lineCount * m.canvas.height;
        var boxMinHeight = SCROLLBOX_MIN_HEIGHT;

        if (boxHeight < boxMinHeight)
            boxHeight = boxMinHeight;

        var topPer = 0;

        if (lineCount !== m.visibleLineCount)
            topPer = m.visibleTop / (lineCount - m.visibleLineCount);

        var x = m.canvas.width - SCROLLBOX_WIDTH;
//      var y = m.visibleTop / lineCount * m.canvas.height;
        var y = (m.canvas.height - boxHeight) * topPer;

        return {
            left:   x,
            top:    y,
            right:  x + SCROLLBOX_WIDTH,
            bottom: y + boxHeight,

            width:  function() {return (this.right  - this.left);},
            height: function() {return (this.bottom - this.top);},

            ptInRect: function(x, y)
            {
                if (x < this.left || this.right < x)
                    return false;

                if (y < this.top || this.bottom < y)
                    return false;

                return true;
            }
        };
    }

    // 最初の可視行を更新
    function updateVisibleTop(view, removeCount)
    {
        var m = view.member;
        var lineCount = m.buffer.getCount();

        if (0 < removeCount)
        {
            m.visibleTop -= removeCount;

            if (m.visibleTop < 0)
                m.visibleTop = 0;
        }

        // 直近のログを表示している場合は、引き続き直近が表示されるようにする
        if (m.visibleTop === lineCount - m.visibleLineCount - 1)
            m.visibleTop =   lineCount - m.visibleLineCount;
    }

    // 最初の可視行を移動
    function moveVisibleTop(view, offset)
    {
        var m = view.member;

        m.visibleTop += offset;
        normalizeVisibleTop(view);
    }

    // 最初の可視行を正常化正常化
    function normalizeVisibleTop(view)
    {
        var m = view.member;

        if (m.visibleTop < 0)
            m.visibleTop = 0;

        var limit = m.buffer.getCount() - m.visibleLineCount;

        if (limit < 0)
            limit = 0;

        if (m.visibleTop > limit)
            m.visibleTop = limit;
    }

    // キー入力処理
    function keydownView(view, event)
    {
        if (focusView !== view)
            return true;

        var m = view.member;
        var keycode = event.keyCode;
        var offset = 0;

        switch (keycode)
        {
        case 38:     // ↑
            offset = -1;
            break;

        case 40:     // ↓
            offset = +1;
            break;

        case 33:     // PageUp
            offset = -m.visibleLineCount;
            break;

        case 34:     // PageDown
            offset = +m.visibleLineCount;
            break;

        case 36:     // Home
            offset = -m.buffer.getCount();
            break;

        case 35:     // End
            offset = +m.buffer.getCount();
            break;

        default:
            return true;
        }

        if (offset !== 0)
            scrollView(view, offset);

        return false;
    }

    // スクロール
    function scrollView(view, offset)
    {
        moveVisibleTop(view, offset);
        drawView(view);
    };

    // 名前空間生成
    if (exports.slog === undefined)
        exports.slog = {};

    if (exports.slog.utils === undefined)
        exports.slog.utils = {};

    // 空バッファ
    exports.slog.utils.emptyBuffer =
    {
        add:      function(data)  {return 0;},
        get:      function(index) {return null;},
        getCount: function()      {return 0;}
    };

    // 普通のバッファ
    exports.slog.utils.Buffer = function()
    {
        this.buffer = [];
    };

    exports.slog.utils.Buffer.prototype =
    {
        add:      function(data)  {       this.buffer[this.buffer.length] = data; return 0;},
        get:      function(index) {return this.buffer[index];},
        getCount: function()      {return this.buffer.length;}
    };

    // リングバッファ
    exports.slog.utils.RingBuffer = function(bufferCount)
    {
        if (bufferCount === undefined)
            bufferCount = 0;

        this.buffer = new Array(bufferCount);
        this.count = 0;     // 追加データ数
    };

    exports.slog.utils.RingBuffer.prototype =
    {
        // データ追加
        // 除去（上書き）された行数を返す
        add: function(data)
        {
            var lastIndex = (this.count % this.buffer.length);

            // リングバッファの最後尾にデータ追加
            this.buffer[lastIndex] = data;
            this.count++;

            return (this.count <= this.buffer.length ? 0 : 1);
        },

        // データ取得
        get: function(index)
        {
            if (this.buffer.length < this.count)
                index +=             this.count;

            index %= this.buffer.length;
            return   this.buffer[index];
        },

        // データ数取得
        // バッファ行数と追加データ数の小さい方を返す
        getCount: function()
        {
            return Math.min(this.buffer.length, this.count);
        }
    };

    // Unicode文字列をUTF-8にした場合のバイト数を取得
    exports.slog.utils.getStringBytes = function(str)
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
    };

    // Unicode文字列をUTF-8でDataViewに設定
    exports.slog.utils.setStringToDataView = function(dataView, offset, str)
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

    // canvasにシーケンスログビューのメソッドを追加
    exports.slog.utils.textView = function(view)
    {
        if (focusView === null)
            focusView = view;

        initView(view);

        view.setBuffer = function(buffer)
        {
            this.member.buffer = buffer;
        }

        view.onUpdateBuffer = function(flag, msg, removeCount)
        {
            updateVisibleTop(this, removeCount);
//          drawView(this);
        };

        view.draw = function()
        {
            drawView(this);
        };
    };
})(this);
