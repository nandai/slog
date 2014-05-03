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

(function(exports)
{
    var STEP_IN =  0;
    var STEP_OUT = 1;
    var MESSAGE =  2;

    var DEBUG = 0;  // デバッグ
    var INFO =  1;  // 情報
    var WARN =  2;  // 警告
    var ERROR = 3;  // エラー

    var INIT =      -1;
    var CONNECTING = 0;
    var OPEN =       1;
    var CLOSED =     3;

    var SequenceLogClient = function()
    {
        this.readyState = INIT;
        this.ws;
        this.fileName;
        this.logLevel;
        this.userName;
        this.passwd;
        this.seqNo = 0;

        this.itemList = []; // 接続が完了する前に出力されたログを貯めておく
    };

    SequenceLogClient.prototype =
    {
        setConfig: function(serviceAddr, fileName, logLevel, userName, passwd)
        {
            this.fileName = fileName;
            this.userName = userName;
            this.passwd = passwd;

            if (logLevel === 'ALL')   this.logLevel = DEBUG - 1;
            if (logLevel === 'DEBUG') this.logLevel = DEBUG;
            if (logLevel === 'INFO')  this.logLevel = INFO;
            if (logLevel === 'WARN')  this.logLevel = WARN;
            if (logLevel === 'ERROR') this.logLevel = ERROR;
            if (logLevel === 'NONE')  this.logLevel = ERROR + 1;

            this.setServiceAddress(serviceAddr);
        },

        setServiceAddress: function(address)
        {
            if (this.logLevel === ERROR + 1)
                return;

            var self = this;
            var WebSocketClient = require('websocket').client;
            var client = new WebSocketClient();
            this.readyState = CONNECTING;

            client.connect(address + '/outputLog');
            client.on('connect', function(connection)
            {
                self.ws = connection;
                self.readyState = OPEN;

                var fileNameLen = self.getStringBytes(self.fileName) + 1;
                var userNameLen = self.getStringBytes(self.userName) + 1;
                var passwdLen =   self.getStringBytes(self.passwd)   + 1;

                var buffer = new ArrayBuffer(
                    4 +
                    4 + userNameLen +
                    4 + passwdLen +
                    4 + fileNameLen +
                    4);

                var dataView = new DataView(buffer);
                var pos = 0;

                // プロセスID（1固定）
                dataView.setUint32(pos, 1);
                pos += 4;

                // ユーザー名
                dataView.setUint32(pos, userNameLen);
                pos += 4;

                self.setStringToDataView(dataView, pos, self.userName);
                pos += userNameLen;

                // パスワード
                dataView.setUint32(pos, passwdLen);
                pos += 4;

                self.setStringToDataView(dataView, pos, self.passwd);
                pos += passwdLen;

                // シーケンスログファイル名
                dataView.setUint32(pos, fileNameLen);
                pos += 4;

                self.setStringToDataView(dataView, pos, self.fileName);
                pos += fileNameLen;

                // ログレベル
                dataView.setUint32(pos, self.logLevel);
                pos += 4;

                // 送信
                self.ws.sendBytes(self.toBuffer(dataView.buffer));
                self.sendAllItems();

                self.ws.on('message', function(e)
                {
//                  var dataView = new DataView(e.data);
//                  var seqNo = dataView.getInt32(0);
                });

                self.ws.on('error', function(error)
                {
                    self.readyState = CLOSED;
                    console.error('error slog WebSocket');
                });

                self.ws.on('close', function()
                {
                    self.readyState = CLOSED;
                    console.info('close slog WebSocket');
                });
            });

            client.on('connectFailed', function(error)
            {
                self.readyState = CLOSED;
                console.error('connect failed slog WebSocket');
            });
        },

        getSequenceNo: function()
        {
            this.seqNo++;
            return this.seqNo;
        },

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

        setStringToDataView: function(dataView, offset, str)
        {
            var len = str.length;
            var pos = offset;

            for (var i = 0; i < len; i++)
            {
                var c = str.charCodeAt(i);

                if (c <= 0x7F)
                {
                    if (dataView)
                    {
                        dataView.setUint8(pos, c);
                    }
                    pos += 1;
                }

                else if (c <= 0x07FF)
                {
                    if (dataView)
                    {
                        dataView.setUint8(pos + 0, 0xC0 | (c >>> 6));
                        dataView.setUint8(pos + 1, 0x80 | (c & 0x3F));
                    }
                    pos += 2;
                }

                else if (c <= 0xFFFF)
                {
                    if (dataView)
                    {
                        dataView.setUint8(pos + 0, 0xE0 |  (c >>> 12));
                        dataView.setUint8(pos + 1, 0x80 | ((c >>>  6) & 0x3F));
                        dataView.setUint8(pos + 2, 0x80 |  (c         & 0x3F));
                    }
                    pos += 3;
                }

                else
                {
                    if (dataView)
                    {
                        dataView.setUint8(pos + 0, 0xF0 |  (c >>> 18));
                        dataView.setUint8(pos + 1, 0x80 | ((c >>> 12) & 0x3F));
                        dataView.setUint8(pos + 2, 0x80 | ((c >>>  6) & 0x3F));
                        dataView.setUint8(pos + 3, 0x80 |  (c         & 0x3F));
                    }
                    pos += 4;
                }
            }

            return (pos - offset);
        },

        toBuffer: function(ab)
        {
            var buffer = new Buffer(ab.byteLength);
            var view = new Uint8Array(ab);

            for (var i = 0; i < buffer.length; ++i)
                buffer[i] = view[i];

            return buffer;
        },

        sendItem: function(item)
        {
            if (this.logLevel === ERROR + 1)
                return;

            if (this.readyState === CONNECTING)
            {
                this.itemList[this.itemList.length] = item;
                return;
            }

            if (this.readyState !== OPEN)
                return;

            var dataView = null;

            for (var i = 0; i < 2; i++)
            {
                var pos = 0;
                var posSave = 0;
                var len = 0;

                // レコード長
                pos += 2;

                // シーケンス番号
                if (dataView) {
                    dataView.setUint32(pos, item.seqNo);
                }
                pos += 4;

                // 日時
                if (dataView) {
                    dataView.setUint8( pos + 0, item.dateTime[7]);
                    dataView.setUint8( pos + 1, item.dateTime[6]);
                    dataView.setUint8( pos + 2, item.dateTime[5]);
                    dataView.setUint8( pos + 3, item.dateTime[4]);
                    dataView.setUint8( pos + 4, item.dateTime[3]);
                    dataView.setUint8( pos + 5, item.dateTime[2]);
                    dataView.setUint8( pos + 6, item.dateTime[1]);
                    dataView.setUint8( pos + 7, item.dateTime[0]);
                }
                pos += 8;

                // シーケンスログアイテム種別
                if (dataView) {
                    dataView.setUint8(pos, item.type);
                }
                pos += 1;

                // スレッド ID（1 固定）
                if (dataView) {
                    dataView.setUint32(pos, 1);
                }
                pos += 4;

                switch (item.type)
                {
                case STEP_IN:
                    // クラス名
                    if (dataView) {
                        // ID は 0 固定
                        dataView.setUint32(pos, 0);
                    }
                    pos += 4;

                    posSave = pos;
                    pos += 2;       // クラス名の長さを格納する領域２バイト分空けておく）

                    len = this.setStringToDataView(dataView, pos, item.className);
                    pos += len;

                    if (dataView)
                        dataView.setUint16(posSave, len);

                    // 関数名
                    if (dataView) {
                        // ID は 0 固定
                        dataView.setUint32(pos, 0);
                    }
                    pos += 4;

                    posSave = pos;
                    pos += 2;       // 関数名の長さを格納する領域２バイト分空けておく）

                    len = this.setStringToDataView(dataView, pos, item.funcName);
                    pos += len;

                    if (dataView)
                        dataView.setUint16(posSave, len);

                    break;

                case STEP_OUT:
                    break;

                case MESSAGE:
                    // メッセージ
                    if (dataView) {
                        // ログレベル
                        dataView.setUint8(pos, item.level);
                    }
                    pos += 1;

                    if (dataView) {
                        // ID は 0 固定
                        dataView.setUint32(pos, 0);
                    }
                    pos += 4;

                    posSave = pos;
                    pos += 2;       // メッセージの長さを格納する領域２バイト分空けておく）

                    len = this.setStringToDataView(dataView, pos, item.message);
                    pos += len;

                    if (dataView)
                        dataView.setUint16(posSave, len);

                    break;
                }

                // 先頭にレコード長
                if (dataView) {
                    dataView.setUint16(0, pos);
                }

                if (i == 0)
                {
                    var buffer = new ArrayBuffer(pos);
                    dataView = new DataView(buffer);
                }
            }

            // 送信
            this.ws.sendBytes(this.toBuffer(dataView.buffer));
        },

        sendAllItems: function()
        {
            var count = this.itemList.length;
            var i;

            for (i = 0; i < count; i++)
            {
                var item = this.itemList[i];
                this.sendItem(item);
            }

            this.itemList = null;
        }
    };

    var SequenceLogItem = function()
    {
        this.seqNo = 0;         // シーケンス番号                 0: uint32_t
        this.dateTime;          // ログ出力日時                   4: DateTime (uint64_t)
        this.type;              // タイプ                        12: uint32_t
//      this.threadId;          // スレッドID                    16: uint32_t

//      this.classId;           // クラスID                      20: uint32_t
//      this.funcId;            // メソッドID                    24: uint32_t

        this.level = 0;         // ログレベル                    32: uint32_t
//      this.messageId;         // メッセージID                  36: uint32_t

        this.className = '';    // クラス名                      40: char[256]
        this.funcName = '';     // メソッド名                   296: char[256]
        this.message = '';      // メッセージ                   552: char[256]

        this.setCurrentDateTime();
    };

    SequenceLogItem.prototype =
    {
        setCurrentDateTime: function()
        {
            var obj = new Date();

            this.dateTime = new Uint8Array(8);
            this.dateTime[0] =  obj.getUTCMilliseconds()       & 0xFF;
            this.dateTime[1] = (obj.getUTCMilliseconds() >> 8) & 0xFF;
            this.dateTime[2] =  obj.getUTCSeconds();
            this.dateTime[3] =  obj.getUTCMinutes();
            this.dateTime[4] =  obj.getUTCHours();
            this.dateTime[5] =  obj.getUTCDate();
            this.dateTime[6] =  obj.getUTCMonth() + 1;
            this.dateTime[7] = (obj.getUTCFullYear() - 1900);
        }
    };

    // シーケンスログ
    var SequenceLog = function(className, funcName)
    {
        this.seqNo;
        stepIn(this, className, funcName);
    };

    SequenceLog.prototype =
    {
        stepOut: function()
        {
            var item = new SequenceLogItem();
            item.seqNo = this.seqNo;
            item.type = STEP_OUT;

            client.sendItem(item);
        },

        d: function(msg) {message(this, DEBUG, msg);},
        i: function(msg) {message(this, INFO,  msg);},
        w: function(msg) {message(this, WARN,  msg);},
        e: function(msg) {message(this, ERROR, msg);}
    };

    function stepIn(log, className, funcName)
    {
        log.seqNo = client.getSequenceNo();

        var item = new SequenceLogItem();
        item.seqNo = log.seqNo;
        item.type = STEP_IN;
        item.className = className;
        item.funcName = funcName;

        client.sendItem(item);
    }

    function message(log, level, msg)
    {
        var item = new SequenceLogItem();
        item.seqNo = log.seqNo;
        item.type = MESSAGE;
        item.level = level;
        item.message = msg;

        client.sendItem(item);
    }

    // slog登録
    var client = new SequenceLogClient();

    exports.setConfig = function(serviceAddr, fileName, logLevel, userName, passwd)
    {
        client.setConfig(serviceAddr, fileName, logLevel, userName, passwd);
    };

    exports.stepIn = function(className, funcName)
    {
        return new SequenceLog(className, funcName);
    };
})(this);
