// require jquery.js, jquery.mousewheel.js, slog.service.js
(function()
{
    var SCROLLBOX_WIDTH = 16;
    var SCROLLBOX_MIN_HEIGHT = 12;

    var yMouse = 0;
    var orgLogVisibleTop = 0;
    var scrollingView = null;

    var focusView = null;

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

        this.logVisibleTop = 0;                             // 最初の可視行
        this.logVisibleLineCount = 20;                      // 可視行数
        this.logBuffer = null;
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
                    orgLogVisibleTop = m.logVisibleTop;
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
            var lineCount = m.logBuffer.getLineCount();
            var linePixel = (m.canvas.height - boxRect.height()) / (lineCount - m.logVisibleLineCount);

            var offset = ~~((event.clientY - yMouse) / linePixel);

            m.logVisibleTop = orgLogVisibleTop + offset;
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
        m.logVisibleLineCount = ~~(m.logVisibleLineCount);
        normalizeVisibleTop(view);

        // 描画行数
        var visibleLineCount = getVisibleLineCount(view);

        // 行番号描画
        m.ctx.fillStyle = 'lightskyblue';

        for (var i = 0; i < visibleLineCount; i++)
        {
            var msg = (m.logVisibleTop + i + 1) + ': ';
            m.ctx.fillText(msg, x, y);

            y += m.size.height;
        }

        // ログ描画
        x = m.size.width * 6;
        y = 0;

        for (var i = 0; i < visibleLineCount; i++)
        {
            var log = m.logBuffer.get(m.logVisibleTop + i);
            var msg = log.msg;

            m.ctx.fillStyle = colors[log.level];
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
        return Math.min(m.logVisibleLineCount, m.logBuffer.getLineCount());
    }

    // スクロールボックスの矩形座標取得
    function getScrollBoxRect(view)
    {
        var m = view.member;

        var visibleLineCount = getVisibleLineCount(view);
        var lineCount = m.logBuffer.getLineCount();
        var boxHeight = visibleLineCount / lineCount * m.canvas.height;
        var boxMinHeight = SCROLLBOX_MIN_HEIGHT;

        if (boxHeight < boxMinHeight)
            boxHeight = boxMinHeight;

        var topPer = 0;

        if (lineCount !== m.logVisibleLineCount)
            topPer = m.logVisibleTop / (lineCount - m.logVisibleLineCount);

        var x = m.canvas.width - SCROLLBOX_WIDTH;
//      var y = m.logVisibleTop / lineCount * m.canvas.height;
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
    function updateVisibleTop(view)
    {
        var m = view.member;
        var bufferLineCount = m.logBuffer.getBufferLineCount();
        var lineCount =       m.logBuffer.getLineCount();

        if (lineCount < m.logVisibleLineCount)
        {
            // ログの総行数が可視行数未満なのでなにもしない
            return;
        }

        if (m.logVisibleTop === bufferLineCount - m.logVisibleLineCount)
        {
            // 既に直近のログが表示される位置にいるのでなにもしない
            return;
        }

        if (lineCount <= bufferLineCount)
        {
            // ログの総行数がバッファ行数以下の場合
            if (m.logVisibleTop === lineCount - m.logVisibleLineCount - 1)
            {
                // 直近のログが表示されるようにする
                m.logVisibleTop =   lineCount - m.logVisibleLineCount;
                return;
            }
        }

        if (bufferLineCount < lineCount)
        {
            // 直近のログを表示していない（見ていない）ときはスクロールされないようにする
            m.logVisibleTop--;

            if (m.logVisibleTop < 0)
                m.logVisibleTop = 0;
        }
    }

    // 最初の可視行を移動
    function moveVisibleTop(view, offset)
    {
        var m = view.member;

        m.logVisibleTop += offset;
        normalizeVisibleTop(view);
    }

    // 最初の可視行を正常化正常化
    function normalizeVisibleTop(view)
    {
        var m = view.member;

        if (m.logVisibleTop < 0)
            m.logVisibleTop = 0;

        var limit = m.logBuffer.getLineCount() - m.logVisibleLineCount;

        if (limit < 0)
            limit = 0;

        if (m.logVisibleTop > limit)
            m.logVisibleTop = limit;
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
            offset = -m.logVisibleLineCount;
            break;

        case 34:     // PageDown
            offset = +m.logVisibleLineCount;
            break;

        case 36:     // Home
            offset = -m.logBuffer.getBufferLineCount();
            break;

        case 35:     // End
            offset = +m.logBuffer.getBufferLineCount();
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

    // canvasにシーケンスログビューのメソッドを追加
    slog.service.defaultLogView = function(view)
    {
        if (focusView === null)
            focusView = view;

        initView(view);

        view.setLogBuffer = function(logBuffer)
        {
            this.member.logBuffer = logBuffer;
        }

        view.onUpdateLog = function(level, msg)
        {
            updateVisibleTop(this);
            drawView(this);
        };

        view.draw = function()
        {
            drawView(this);
        };
   };
})(this);
