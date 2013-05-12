$(function()
{
    // シーケンスログファイル一覧更新
    function updateLogFileList(data)
    {
        var list = '';

        $.each(data, function(i, obj)
        {
            list = list +
                '<tr>' +
                    '<td>'               + obj.creationTime  + '</td>' +
                    '<td>'               + obj.lastWriteTime + '</td>' +
                    '<td><a href="#">'   + obj.canonicalPath + '</a></td>' +
                    '<td align="right">' + obj.size          + '</td>' +
                '</tr>';
        });

        $('#logFileList').find('tr:gt(0)').remove();
        $('#logFileList').append(list);

        $('a[href=#]').click(function()
        {
            slog.openLog($(this).text());
            return false;
        });
    }

    // ブラウザサイズ変更時のハンドラ
    function onSize(view)
    {
//      var height = $(window).height();
//      view.member.logVisibleLineCount = height / view.member.size.height / 2;

        var canvas =    view.get(0);
        canvas.width =  view.parent().width() * 95 / 100;
        canvas.height = view.member.size.height * view.member.logVisibleLineCount;

        view.draw();
    }

    // シーケンスログビュー設定
    function settingLogView(logView)
    {
        slog.service.defaultLogView(logView);
        slog.service.addLogView(logView);

        $(window).resize(function() {onSize(logView);});
        onSize(logView);
    }

    // シーケンスログサービス設定
    var domain = location.href.split('/')[2];
    slog.service.init(domain, updateLogFileList);

    // デフォルトビュー設定
    var logView = $('#logView');
    settingLogView(logView);

    // テスト用
//  var logView2 = $('#logView2');
//  settingLogView(logView2);
});
