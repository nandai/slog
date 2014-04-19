$(function()
{
    'use strict';

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
            var tr = $(this).closest('tr');
            var i = tr.index() - 1;
            slog.service.openLog(i, $(this).text());
            return false;
        });
    }

    // ブラウザサイズ変更時のハンドラ
    function onSize(view)
    {
        var canvas =    view.get(0);
        canvas.width =  view.parent().width() * 95 / 100;
        canvas.height = view.member.size.height * view.member.visibleLineCount;

        view.draw();
    }

    // シーケンスログビュー設定
    function settingLogView(logView)
    {
        slog.utils.textView(logView);
        slog.service.addLogView(logView);

        $(window).resize(function() {onSize(logView);});
        onSize(logView);
    }

    // シーケンスログサービス設定
    var domain = location.href.split('/')[2];
    var buffer = new slog.utils.RingBuffer(2000);
    slog.service.init(domain, buffer, updateLogFileList);

    // デフォルトビュー設定
    var logView = $('#logView');
    settingLogView(logView);

    // テスト用
//  var logView2 = $('#logView2');
//  settingLogView(logView2);

    $('#account').on('click', function()
    {
        location.href = 'account.html';
    });

    $('#logout').on('click', function()
    {
//      location.href = 'logout';
        location.href = 'logoff';
    });
});
