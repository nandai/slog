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
                    '<td align="center">' + obj.creationTime  + '</td>' +
                    '<td align="center">' + obj.lastWriteTime + '</td>' +
                    '<td><a href="#">'    + obj.canonicalPath + '</a></td>' +
                    '<td align="right">'  + obj.size          + '</td>' +
                '</tr>';
        });

        $('#logFileList tr').remove();
        $('#logFileList').append(list);

        $('a[href=#]').click(function()
        {
            var tr = $(this).closest('tr');
            var i = tr.index();
            slog.service.openLog(i, $(this).text());
            return false;
        });
    }

    // ブラウザサイズ変更時のハンドラ
    function onSize(view)
    {
        var canvas =    view.get(0);
        view.member.visibleLineCount = (window.innerHeight - 130) / view.member.size.height;

        canvas.width =  view.parent().width() * 98 / 100;
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

    $('#showLogFileList').on('click', function()
    {
        var body = $('body');
        var logFileListPanelHeight = body.height() - 120;

        $('#logFileListPanel').height(logFileListPanelHeight);

        $('#mainPanel')
            .css('top',  (body.height() - logFileListPanelHeight)  / 2)
            .css('left', (body.width()  - $('#mainPanel').width()) / 2);

        $('#modalPanel').fadeIn('fast');
        $('#mainPanel'). fadeIn('fast');
    });

    $('#modalPanel').on('click', function()
    {
        $('#modalPanel').fadeOut('fast');
        $('#mainPanel').fadeOut('fast');
    });

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
