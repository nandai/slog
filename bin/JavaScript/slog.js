/*
 * Copyright (C) 2011-2013 printf.jp
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
    var SequenceLogClient = function()
    {
        this.ws;
        this.name;
        this.rootFlag = this.ROOT;
        this.seqNo = 0;

        this.itemList = []; // 接続が完了する前に出力されたログを貯めておく
    };

    SequenceLogClient.prototype =
    {
        KEEP: 0,
        ROOT: 3,

        setFileName: function(name)
        {
            this.name = name;
        },

        setServiceAddress: function(address, port)
        {
            var self = this;
            this.ws = new WebSocket('ws://' + address + ':' + port + '/outputLog');
            this.ws.binaryType = 'arraybuffer';

            this.ws.onopen = function()
            {
                var len = self.getStringBytes(self.name) + 1;
                var buffer = new ArrayBuffer(4 + 4 + len);
                var dataView = new DataView(buffer);

                dataView.setUint32(0, 1);       // プロセスID
                dataView.setUint32(4, len);     // ファイル名の長さ
                self.setStringToDataView(dataView, 8, self.name);

                self.ws.send(dataView.buffer);
                self.sendAllItems();
            };

            this.ws.onmessage = function(e)
            {
//              var dataView = new DataView(e.data);
//              var seqNo = dataView.getInt32(0); 
            };

            this.ws.onerror = function()
            {
            };

            this.ws.onclose = function()
            {
            };
        },

        enableOutput: function(enable)
        {
            this.rootFlag = (enable ? this.ROOT : this.KEEP);
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
        },

        sendItem: function(item)
        {
            if (this.ws.readyState === WebSocket.CONNECTING)
            {
                this.itemList[this.itemList.length] = item;
                return;
            }

            if (this.ws.readyState !== WebSocket.OPEN)
                return;

            var buffer = new ArrayBuffer(824);
            var dataView = new DataView(buffer);

            dataView.setUint32(  0,  item.seqNo, true);
            dataView.setUint8(   4,  item.dateTime[0]);
            dataView.setUint8(   5,  item.dateTime[1]);
            dataView.setUint8(   6,  item.dateTime[2]);
            dataView.setUint8(   7,  item.dateTime[3]);
            dataView.setUint8(   8,  item.dateTime[4]);
            dataView.setUint8(   9,  item.dateTime[5]);
            dataView.setUint8(  10,  item.dateTime[6]);
            dataView.setUint8(  11,  item.dateTime[7]);
            dataView.setUint32( 12,  item.type, true);
            dataView.setUint32( 16,  1, true);

            dataView.setUint32( 20,  0, true);
            dataView.setUint32( 24,  0, true);

            dataView.setUint32( 28,  item.outputFlag, true);

            dataView.setUint32( 32,  item.level, true);
            dataView.setUint32( 36,  0, true);

            this.setStringToDataView(dataView,  40, item.className);
            this.setStringToDataView(dataView, 296, item.funcName);
            this.setStringToDataView(dataView, 552, item.message);

            dataView.setUint32(808,  0);
            dataView.setUint32(812,  0);
            dataView.setUint32(816,  0);
            dataView.setUint32(820,  0);

            this.ws.send(buffer);
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

        this.outputFlag;        // 出力フラグ                    28: uint32_t

        this.level = 0;         // ログレベル                    32: uint32_t
//      this.messageId;         // メッセージID                  36: uint32_t

        this.className = '';    // クラス名                      40: char[256]
        this.funcName = '';     // メソッド名                   296: char[256]
        this.message = '';      // メッセージ                   552: char[256]

//      this.prev;              // 前のシーケンスログアイテム   808: uint64_t
//      this.next;              // 次のシーケンスログアイテム   816: uint64_t

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

    // 定数定義
    var STEP_IN =  0;
    var STEP_OUT = 1;
    var MESSAGE =  2;

    var DEBUG = 0;  // デバッグ
    var INFO =  1;  // 情報
    var WARN =  2;  // 警告
    var ERROR = 3;  // エラー

    // シーケンスログ
    var SequenceLog = function(className, funcName)
    {
        this.seqNo;
        this.outputFlag;

        stepIn(this, className, funcName);
    };

    SequenceLog.prototype =
    {
        stepOut: function()
        {
            var item = new SequenceLogItem();
            item.seqNo = this.seqNo;
            item.type = STEP_OUT;
            item.outputFlag = this.outputFlag;

            client.sendItem(item);
        },

        d: function(msg) {message(this, DEBUG, msg);},
        i: function(msg) {message(this, INFO,  msg);},
        w: function(msg) {message(this, WARN,  msg);},
        e: function(msg) {message(this, ERROR, msg);}
    };

    function stepIn(log, className, funcName)
    {
        log.seqNo =      client.getSequenceNo();
        log.outputFlag = client.rootFlag;

        var item = new SequenceLogItem();
        item.seqNo = log.seqNo;
        item.type = STEP_IN;
        item.outputFlag = log.outputFlag;
        item.className = className;
        item.funcName = funcName;

        client.sendItem(item);
    }

    function message(log, level, msg)
    {
        var item = new SequenceLogItem();
        item.seqNo = log.seqNo;
        item.type = MESSAGE;
        item.outputFlag = log.outputFlag;
        item.level = level;
        item.message = msg;

        client.sendItem(item);
    }

    // slog登録
    var client = new SequenceLogClient();

    exports.slog =
    {
        setFileName:       function(name)          {client.setFileName(name);},
        setServiceAddress: function(address, port) {client.setServiceAddress(address, port);},
        enableOutput:      function(enable)        {client.enableOutput(enable);},

        stepIn: function(className, funcName) {return new SequenceLog(className, funcName);}
    };
})(this);
