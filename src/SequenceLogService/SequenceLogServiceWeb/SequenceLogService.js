$(function()
{
    // ブラウザサイズ変更時のハンドラ
    function onSize()
    {
        var canvas =    $('#logViewer').get(0);
        canvas.width =  $('#logViewer').parent().width() * 95 / 100;
        canvas.height = $.sequenceLogList.size.height * $.sequenceLogList.logVisibleLineCount;

        $.sequenceLogList.draw();
    }

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
            $.sequenceLogList.openLog($(this).text());
            return false;
        });
    }

    // 初期設定
    var domain = location.href.split('/')[2];
    $.sequenceLogList.init(domain, updateLogFileList, 'logViewer');

    $(window).bind('resize', onSize);
    onSize();
});
