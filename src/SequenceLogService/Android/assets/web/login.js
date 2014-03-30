$(function()
{
    var check = true;

    $('form').submit(function(event)
    {
        if (check == false)
            return;

        // HTMLでの送信をキャンセル
        event.preventDefault();
        
        // 操作対象のフォーム要素を取得
        var $form = $(this);
        
        // 送信ボタンを取得
        // （後で使う: 二重送信を防止する。）
//      var $button = $form.find('button');

        $('#waiting').html('<img src="/wait.gif" width="24" height="24" alt="Wait..." />');

        // 送信
        $.ajax(
        {
            type: 'POST',
            dataType: 'json',
            data: $form.serialize() + '&phase=validate',
            timeout: 10000,

            // 送信前
            beforeSend: function(xhr, settings)
            {
                // ボタンを無効化し、二重送信を防止
//              $button.attr('disabled', true);
                $('#message').html('');
            },

            // 応答後
            complete: function(xhr, textStatus)
            {
                // ボタンを有効化し、再送信を許可
//              $button.attr('disabled', false);

                $('#waiting').html('');
            },

            // 通信成功時の処理
            success: function(result, textStatus, xhr)
            {
                var message = '';
                var sep = '';

                $.each(result, function(key, value)
                {
//                  $('#' + key).attr('class', 'l_Cel_err');
                    message += sep + value + '<br />';
                    sep = '<br />';
                });

                if (message.length == 0)
                {
                    check = false;
                    $form.trigger('submit');
                }
                else
                {
                    $('#message').html('<br />' + message);
                }
            },

            // 通信失敗時の処理
            error: function(xhr, textStatus, error)
            {
                if (navigator.language === 'ja')
                    $('#message').html('<br />サーバーに接続できませんでした。');
                else
                    $('#message').html('<br />It was not possible to connect to the server.');
            }
        });
    });
});
