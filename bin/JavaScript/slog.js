/*
* Copyright (C) 2011-2014 printf.jp
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/
/**
* @namespace slog
*/
var slog;
(function (slog) {
    var STEP_IN = 0;
    var STEP_OUT = 1;
    var MESSAGE = 2;

    var DEBUG = 0;
    var INFO = 1;
    var WARN = 2;
    var ERROR = 3;

    var INIT = -1;
    var CONNECTING = 0;
    var OPEN = 1;
    var CLOSED = 3;

    /**
    * シーケンスログクライアント（singleton）
    *
    * @class   SequenceLogClient
    */
    var SequenceLogClient = (function () {
        function SequenceLogClient() {
            /**
            * WebSocketの状態
            */
            this.readyState = INIT;
            /**
            * シーケンスNo
            */
            this.seqNo = 0;
            /**
            * 接続が完了する前に出力されたログを貯めておくリスト
            */
            this.itemList = [];
            /**
            * itemListの現在位置
            */
            this.itemListPos = 0;
        }
        /**
        * ログ出力設定を行います。
        *
        * @method  setConfig
        *
        * @param   address     Sequence Log Serviceへの接続情報
        * @param   fileName    ログの出力ファイル名
        * @param   logLevel    ログレベル
        * @param   userName    ユーザー名
        * @param   passwd      パスワード
        *
        * @return  なし
        */
        SequenceLogClient.prototype.setConfig = function (address, fileName, logLevel, userName, passwd) {
            if (logLevel === 'ALL')
                this.logLevel = DEBUG - 1;
            if (logLevel === 'DEBUG')
                this.logLevel = DEBUG;
            if (logLevel === 'INFO')
                this.logLevel = INFO;
            if (logLevel === 'WARN')
                this.logLevel = WARN;
            if (logLevel === 'ERROR')
                this.logLevel = ERROR;
            if (logLevel === 'NONE')
                this.logLevel = ERROR + 1;

            if (this.logLevel === ERROR + 1)
                return;

            // 接続
            var self = this;
            this.ws = new WebSocket(address + '/outputLog');
            this.ws.binaryType = 'arraybuffer';
            this.readyState = CONNECTING;

            this.ws.onopen = function () {
                self.readyState = OPEN;

                var fileNameLen = self.getStringBytes(fileName) + 1;
                var userNameLen = self.getStringBytes(userName) + 1;
                var passwdLen = self.getStringBytes(passwd) + 1;

                var array = new Uint8Array(4 + 4 + userNameLen + 4 + passwdLen + 4 + fileNameLen + 4);
                var pos = 0;

                // プロセスID
                var pid = 0;
                array[pos++] = (pid >> 24) & 0xFF;
                array[pos++] = (pid >> 16) & 0xFF;
                array[pos++] = (pid >> 8) & 0xFF;
                array[pos++] = pid & 0xFF;

                // ユーザー名
                array[pos++] = (userNameLen >> 24) & 0xFF;
                array[pos++] = (userNameLen >> 16) & 0xFF;
                array[pos++] = (userNameLen >> 8) & 0xFF;
                array[pos++] = userNameLen & 0xFF;

                self.setStringToUint8Array(array, pos, userName);
                pos += userNameLen;

                // パスワード
                array[pos++] = (passwdLen >> 24) & 0xFF;
                array[pos++] = (passwdLen >> 16) & 0xFF;
                array[pos++] = (passwdLen >> 8) & 0xFF;
                array[pos++] = passwdLen & 0xFF;

                self.setStringToUint8Array(array, pos, passwd);
                pos += passwdLen;

                // シーケンスログファイル名
                array[pos++] = (fileNameLen >> 24) & 0xFF;
                array[pos++] = (fileNameLen >> 16) & 0xFF;
                array[pos++] = (fileNameLen >> 8) & 0xFF;
                array[pos++] = fileNameLen & 0xFF;

                self.setStringToUint8Array(array, pos, fileName);
                pos += fileNameLen;

                // ログレベル
                array[pos++] = (self.logLevel >> 24) & 0xFF;
                array[pos++] = (self.logLevel >> 16) & 0xFF;
                array[pos++] = (self.logLevel >> 8) & 0xFF;
                array[pos++] = self.logLevel & 0xFF;

                // 送信
                self.ws.send(array.buffer);
                self.sendAllItems();
            };

            this.ws.onmessage = function (e) {
                //              var array = new DataView(e.data);
                //              var seqNo = array.getInt32(0);
            };

            this.ws.onerror = function () {
                self.readyState = CLOSED;
                console.error('error slog WebSocket');
            };

            this.ws.onclose = function () {
                self.readyState = CLOSED;
                console.info('close slog WebSocket');
            };
        };

        /**
        * シーケンスNoを取得します。
        *
        * @method  getSequenceNo
        *
        * @return  シーケンスNo
        */
        SequenceLogClient.prototype.getSequenceNo = function () {
            this.seqNo++;
            return this.seqNo;
        };

        /**
        * 文字列のバイト数を取得します。
        *
        * @method  getStringBytes
        *
        * @param   str バイト数を取得する文字列（UTF-8）
        *
        * @return  文字列のバイト数
        */
        SequenceLogClient.prototype.getStringBytes = function (str) {
            var len = str.length;
            var bytes = 0;

            for (var i = 0; i < len; i++) {
                var c = str.charCodeAt(i);

                if (c <= 0x7F) {
                    bytes += 1;
                } else if (c <= 0x07FF) {
                    bytes += 2;
                } else if (c <= 0xFFFF) {
                    bytes += 3;
                } else {
                    bytes += 4;
                }
            }

            return bytes;
        };

        /**
        * 文字列をUint8Arrayに変換します。
        *
        * @method  setStringToUint8Array
        *
        * @param   array   変換先
        * @param   offset  arrayへのオフセット
        * @param   str     Uint8Arrayに変換する文字列（UTF-8）
        *
        * @return  文字列のバイト数
        */
        SequenceLogClient.prototype.setStringToUint8Array = function (array, offset, str) {
            var len = str.length;
            var pos = offset;

            for (var i = 0; i < len; i++) {
                var c = str.charCodeAt(i);

                if (c <= 0x7F) {
                    array[pos++] = c;
                } else if (c <= 0x07FF) {
                    array[pos++] = 0xC0 | (c >>> 6);
                    array[pos++] = 0x80 | (c & 0x3F);
                } else if (c <= 0xFFFF) {
                    array[pos++] = 0xE0 | (c >>> 12);
                    array[pos++] = 0x80 | ((c >>> 6) & 0x3F);
                    array[pos++] = 0x80 | (c & 0x3F);
                } else {
                    array[pos++] = 0xF0 | (c >>> 18);
                    array[pos++] = 0x80 | ((c >>> 12) & 0x3F);
                    array[pos++] = 0x80 | ((c >>> 6) & 0x3F);
                    array[pos++] = 0x80 | (c & 0x3F);
                }
            }

            return (pos - offset);
        };

        /**
        * ログ出力可能かどうか調べます。
        *
        * @method  canOutput
        *
        * @return  ログ出力が可能な場合はtrue
        */
        SequenceLogClient.prototype.canOutput = function () {
            if (this.logLevel === ERROR + 1)
                return false;

            if (this.readyState !== CONNECTING && this.readyState !== OPEN) {
                return false;
            }

            return true;
        };

        /**
        * SequenceLogItemのバイト数を取得します。
        *
        * @method  getItemBytes
        *
        * @param   item    バイト数を取得するSequenceLogItem
        *
        * @return  SequenceLogItemのバイト数
        */
        SequenceLogClient.prototype.getItemBytes = function (item) {
            var pos = 0;
            var len = 0;

            // レコード長
            pos += 2;

            // シーケンス番号
            pos += 4;

            // 日時
            pos += 8;

            // シーケンスログアイテム種別
            pos += 1;

            // スレッド ID（1 固定）
            pos += 4;

            switch (item.type) {
                case STEP_IN:
                    // クラス名
                    pos += 4;

                    pos += 2; // クラス名の長さを格納する領域２バイト分空けておく）

                    len = this.getStringBytes(item.className);
                    pos += len;

                    // 関数名
                    pos += 4;

                    pos += 2; // 関数名の長さを格納する領域２バイト分空けておく）

                    len = this.getStringBytes(item.funcName);
                    pos += len;
                    break;

                case STEP_OUT:
                    break;

                case MESSAGE:
                    // メッセージ
                    pos += 1;

                    pos += 4;

                    pos += 2; // メッセージの長さを格納する領域２バイト分空けておく）

                    len = this.getStringBytes(item.message);
                    pos += len;
                    break;
            }

            return pos;
        };

        /**
        * SequenceLogItemをUint8Arrayに変換します。
        *
        * @method  setStringToUint8Array
        *
        * @param   array   変換先
        * @param   offset  arrayへのオフセット
        * @param   item    Uint8Arrayに変換するSequenceLogItem
        *
        * @return  文字列のバイト数
        */
        SequenceLogClient.prototype.itemToUint8Array = function (array, offset, item) {
            var pos = offset;
            var posSave = 0;
            var len = 0;

            // レコード長
            pos += 2;

            // シーケンス番号
            array[pos++] = (item.seqNo >> 24) & 0xFF;
            array[pos++] = (item.seqNo >> 16) & 0xFF;
            array[pos++] = (item.seqNo >> 8) & 0xFF;
            array[pos++] = item.seqNo & 0xFF;

            // 日時
            var dateTime = item.dateTime;
            array[pos++] = (dateTime.getUTCFullYear() - 1900);
            array[pos++] = dateTime.getUTCMonth() + 1;
            array[pos++] = dateTime.getUTCDate();
            array[pos++] = dateTime.getUTCHours();
            array[pos++] = dateTime.getUTCMinutes();
            array[pos++] = dateTime.getUTCSeconds();
            array[pos++] = (dateTime.getUTCMilliseconds() >> 8) & 0xFF;
            array[pos++] = dateTime.getUTCMilliseconds() & 0xFF;

            // シーケンスログアイテム種別
            array[pos++] = item.type;

            // スレッド ID（1 固定）
            array[pos++] = 0;
            array[pos++] = 0;
            array[pos++] = 0;
            array[pos++] = 1;

            switch (item.type) {
                case STEP_IN:
                    // クラス名
                    // ID は 0 固定
                    array[pos++] = 0;
                    array[pos++] = 0;
                    array[pos++] = 0;
                    array[pos++] = 0;

                    posSave = pos;
                    pos += 2; // クラス名の長さを格納する領域２バイト分空けておく）

                    len = this.setStringToUint8Array(array, pos, item.className);
                    pos += len;

                    array[posSave++] = (len >> 8) & 0xFF;
                    array[posSave++] = len & 0xFF;

                    // 関数名
                    // ID は 0 固定
                    array[pos++] = 0;
                    array[pos++] = 0;
                    array[pos++] = 0;
                    array[pos++] = 0;

                    posSave = pos;
                    pos += 2; // 関数名の長さを格納する領域２バイト分空けておく）

                    len = this.setStringToUint8Array(array, pos, item.funcName);
                    pos += len;

                    array[posSave++] = (len >> 8) & 0xFF;
                    array[posSave++] = len & 0xFF;
                    break;

                case STEP_OUT:
                    break;

                case MESSAGE:
                    // メッセージ
                    // ログレベル
                    array[pos++] = item.level;

                    // ID は 0 固定
                    array[pos++] = 0;
                    array[pos++] = 0;
                    array[pos++] = 0;
                    array[pos++] = 0;

                    posSave = pos;
                    pos += 2; // メッセージの長さを格納する領域２バイト分空けておく）

                    len = this.setStringToUint8Array(array, pos, item.message);
                    pos += len;

                    array[posSave++] = (len >> 8) & 0xFF;
                    array[posSave++] = len & 0xFF;
                    break;
            }

            // 先頭にレコード長
            array[0] = (pos >> 8) & 0xFF;
            array[1] = pos & 0xFF;

            return pos;
        };

        /**
        * SequenceLogItemを送信します。
        *
        * @method  sendItem
        *
        * @param   item    送信するSequenceLogItem
        *
        * @return  なし
        */
        SequenceLogClient.prototype.sendItem = function (item) {
            if (this.readyState === CONNECTING)
                return;

            var size = this.getItemBytes(item);
            var array = new Uint8Array(size);

            this.itemToUint8Array(array, 0, item);
            this.ws.send(array.buffer);
        };

        /**
        * すべてのSequenceLogItemを送信します。
        *
        * @method  sendAllItems
        *
        * @return  なし
        */
        SequenceLogClient.prototype.sendAllItems = function () {
            var itemList = this.itemList;
            var count = itemList.length;

            for (var i = 0; i < count; i++) {
                var item = itemList[i];
                this.sendItem(item);
            }

            this.itemListPos = 0;
        };

        /**
        * SequenceLogItemを取得します。
        *
        * @method  getItem
        *
        * @return  SequenceLogItem
        */
        SequenceLogClient.prototype.getItem = function () {
            if (this.readyState === OPEN)
                this.itemListPos = 0;

            var itemList = this.itemList;
            var count = itemList.length;

            var pos = this.itemListPos++;

            if (pos === count) {
                var item = new SequenceLogItem();
                itemList[count] = item;
            }

            return itemList[pos];
        };
        return SequenceLogClient;
    })();

    /**
    * シーケンスログアイテム
    *
    * @class   SequenceLogItem
    */
    var SequenceLogItem = (function () {
        function SequenceLogItem() {
            /**
            * シーケンスNo
            */
            this.seqNo = 0;
            /**
            * ログ出力日時
            */
            this.dateTime = new Date();
            /**
            * スレッドID
            */
            //      threadId : number;
            /**
            * クラスID
            */
            //      classId : number;
            /**
            * メソッドID
            */
            //      funcId : number;
            /**
            * ログレベル
            */
            this.level = 0;
            /**
            * メッセージID
            */
            //      messageId : number;
            /**
            * クラス名
            */
            this.className = '';
            /**
            * メソッド名
            */
            this.funcName = '';
            /**
            * メッセージ
            */
            this.message = '';
        }
        return SequenceLogItem;
    })();

    /**
    * シーケンスログ
    *
    * @class   SequenceLog
    */
    var SequenceLog = (function () {
        /**
        * コンストラクタ
        *
        * @constructor
        */
        function SequenceLog(className, funcName) {
            if (client.canOutput() === false)
                return;

            this.seqNo = client.getSequenceNo();

            var item = client.getItem();
            item.seqNo = this.seqNo;
            item.type = STEP_IN;
            item.className = className;
            item.funcName = funcName;

            client.sendItem(item);
        }
        /**
        * メソッドのリターンログを出力します。
        *
        * @method  stepOut
        *
        * @return  なし
        */
        SequenceLog.prototype.stepOut = function () {
            if (client.canOutput() === false)
                return;

            var item = client.getItem();
            item.seqNo = this.seqNo;
            item.type = STEP_OUT;

            client.sendItem(item);
        };

        SequenceLog.prototype.d = function (msg) {
            message(this, DEBUG, msg);
        };
        SequenceLog.prototype.i = function (msg) {
            message(this, INFO, msg);
        };
        SequenceLog.prototype.w = function (msg) {
            message(this, WARN, msg);
        };
        SequenceLog.prototype.e = function (msg) {
            message(this, ERROR, msg);
        };
        return SequenceLog;
    })();
    slog.SequenceLog = SequenceLog;

    /**
    * ログメッセージを出力します。
    *
    * @method  message
    *
    * @return  なし
    */
    function message(log, level, msg) {
        if (client.canOutput() === false)
            return;

        var item = client.getItem();
        item.seqNo = log.seqNo;
        item.type = MESSAGE;
        item.level = level;
        item.message = msg;

        client.sendItem(item);
    }

    /**
    * ログ出力設定を行います。
    *
    * @method  setConfig
    *
    * @param   address     Sequence Log Serviceへの接続情報
    * @param   fileName    ログの出力ファイル名
    * @param   logLevel    ログレベル
    *
    * @return  なし
    */
    function setConfig(address, fileName, logLevel) {
        client.setConfig(address, fileName, logLevel, '', '');
    }
    slog.setConfig = setConfig;
    ;

    /**
    * メソッドのコールログを出力します。
    *
    * @method  setConfig
    *
    * @param   address     Sequence Log Serviceへの接続情報
    * @param   fileName    ログの出力ファイル名
    * @param   logLevel    ログレベル
    *
    * @return  なし
    */
    function stepIn(className, funcName) {
        return new SequenceLog(className, funcName);
    }
    slog.stepIn = stepIn;
    ;

    // シーケンスログクライアント生成
    var client = new SequenceLogClient();
})(slog || (slog = {}));
