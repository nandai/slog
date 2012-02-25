/*
 * Copyright (C) 2011 log-tools.net
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

// IEでconstが使えないのでvarに変更
var SL_KEEP =       0;	// シーケンスログの出力をキープする
var SL_OUTPUT_ALL = 1;	// キープ中のシーケンスログも含め出力する
var SL_ALWAYS =     2;	// キープ中のシーケンスログを出力し、さらに配下のシーケンスログは即座に出力する
var SL_ROOT =       3;

var SL_DEBUG = '0';		// デバッグ
var SL_INFO =  '1';		// 情報
var SL_WARN =  '2';		// 警告
var SL_ERROR = '3';		// エラー

var SL_IN =      0;
var SL_OUT =     1;
var SL_MESSAGE = 2;

var SL_IMG_SAVE =        'images/save.png';
var SL_IMG_SAVE_DOWN =   'images/save_down.png';
var SL_IMG_CLEAR =       'images/clear.png';
var SL_IMG_CLEAR_DOWN =  'images/clear_down.png';
var SL_IMG_AUTO =        'images/auto.png';
var SL_IMG_AUTO_DOWN =   'images/auto_down.png';
var SL_IMG_STOP =        'images/stop.png';
var SL_IMG_STOP_DOWN =   'images/stop_down.png';
var SL_IMG_UPDATE =      'images/update.png';
var SL_IMG_UPDATE_DOWN = 'images/update_down.png';
var SL_IMG_CLOSE =       'images/close.png';
var SL_IMG_CLOSE_DOWN =  'images/close_down.png';

function slogShow()
{
	var view = document.getElementById('slogView');
	slogCtrl.show = true;
	slogCtrl.update();

	if (view)
		view.style.display = '';
}

function slogHide()
{
	var view = document.getElementById('slogView');
	slogCtrl.show = false;

	if (view)
		view.style.display = 'none';
}

function slogSetRootFlag(outputFlag)
{
	slogCtrl.rootFlag = outputFlag;
}

// === sequence log controller ===
function SlogController()
{
	this.show = false;

	this.seqNo = 1;
	this.alwaysCount = 0;
	this.logBuffer = new Array();
	this.slogs = new Array();
	this.updateInterval = -1;
	this.rootFlag = SL_KEEP;
	
	this.dragObj = null;
	this.beginX;
	this.beginY;
	this.beginLeft;
	this.beginTop;

	return this;
}

SlogController.prototype =
{
	init : function()
	{
		var d = 2;
		var body = document.getElementsByTagName('body')[0];

		// create log view
		var view = document.createElement('div');
		body.appendChild(view);

		var width =  slogCtrl.getWindowWidth();
		var height = slogCtrl.getWindowHeight();

		view.id = 'slogView';
		view.style.left =   (width  - width  / d - 10) + 'px';
		view.style.top =    (height - height / d - 10) + 'px';
		view.style.width =  (width  / d) + 'px';
		view.style.height = (height / d) + 'px';

		view.innerHTML =
			'<div id="slogViewHeader">' +
				'<img id="slogViewSave"   src="' + SL_IMG_SAVE + '" />' +
				'<img id="slogViewClear"  src="' + SL_IMG_CLEAR + '" />' +
				'<img id="slogViewAuto"   src="' + SL_IMG_AUTO + '" />' +
				'<img id="slogViewStop"   src="' + SL_IMG_STOP + '" />' +
				'<img id="slogViewUpdate" src="' + SL_IMG_UPDATE + '" />' +
				'<img id="slogViewClose"  src="' + SL_IMG_CLOSE + '" />' +
			'</div>' +
			'<div id="slogViewContent"></div>';

		var header = document.getElementById('slogViewHeader');
		header.onmousedown = slogCtrl.onmousedown;
//		header.onmousemove = slogCtrl.onmousemove;
//		header.onmouseup =   slogCtrl.onmouseup;

		var save = document.getElementById('slogViewSave');
		save.onmousedown = function() {this.src = SL_IMG_SAVE_DOWN;}
		save.onmouseup =   function() {this.src = SL_IMG_SAVE;}
		save.onclick = slogCtrl.save;

		var clear = document.getElementById('slogViewClear');
		clear.onmousedown = function() {this.src = SL_IMG_CLEAR_DOWN;}
		clear.onmouseup =   function() {this.src = SL_IMG_CLEAR;}
		clear.onclick = slogCtrl.clear;

		var auto = document.getElementById('slogViewAuto');
		auto.onmousedown = function() {this.src = SL_IMG_AUTO_DOWN;}
		auto.onmouseup =   function() {this.src = SL_IMG_AUTO;}
		auto.onclick = slogCtrl.auto;

		if (slogCtrl.updateInterval == 0)
			auto.style.display = 'none';
		
		var stop = document.getElementById('slogViewStop');
		stop.onmousedown = function() {this.src = SL_IMG_STOP_DOWN;}
		stop.onmouseup =   function() {this.src = SL_IMG_STOP;}
		stop.onclick = slogCtrl.stop;

		if (slogCtrl.updateInterval == -1)
			stop.style.display = 'none';

		var update = document.getElementById('slogViewUpdate');
		update.onmousedown = function() {this.src = SL_IMG_UPDATE_DOWN;}
		update.onmouseup =   function() {this.src = SL_IMG_UPDATE;}
		update.onclick = slogCtrl.update;

		var close = document.getElementById('slogViewClose');
		close.onmousedown = function() {this.src = SL_IMG_CLOSE_DOWN;}
		close.onmouseup =   function() {this.src = SL_IMG_CLOSE;}
		close.onclick = slogHide;

		var content = document.getElementById('slogViewContent');
		var width =  slogCtrl.getWindowWidth();
		var height = slogCtrl.getWindowHeight();

		content.style.width =  (width  / d - 20) + 'px';	// 20 = slogViewContent 10 padding * 2
		content.style.height = (height / d - 28) + 'px';	// 28 = slogViewHeader 20 height + 4 padding * 2

//		slogCtrl.update();

		if (slogCtrl.show == false)
			slogHide();
	},

	createString : function(seqNo, type)
	{
		var now = new Date();
		now.setTime(now.getTime() - (-now.getTimezoneOffset() * 60 * 1000));

		var str = '';

		if (type == SL_MESSAGE)
		{
			var level = arguments[2];

			switch (level)
			{
			case SL_DEBUG:
				str = '<font color="gray">';
				break;

			case SL_INFO:
				str = '<font color="white">';
				break;

			case SL_WARN:
				str = '<font color="yellow">';
				break;

			case SL_ERROR:
				str = '<font color="red">';
				break;
			}
		}

		str += seqNo + ' ' +
			parseInt(now.getFullYear()) + '/' + (now.getMonth() + 1) + '/' + now.getDate() + ' ' + now.getHours() + ':' + now.getMinutes() + ':' + now.getSeconds() + '.' + now.getMilliseconds() + ' ' + type + ' 1';

		if (type == SL_IN)
		{
			str += ' 0 ' + arguments[2] + ' 0 ' + arguments[3];
		}
		else if (type == SL_MESSAGE)
		{
			str += ' ' + arguments[2] + ' 0 ' + arguments[3] + '</font>';
		}
		
		return str;
	},

	output : function(slog, type, str)
	{
		do
		{
			if (type != SL_OUT)
				break;

			if (slog.outputFlag == SL_ALWAYS)
				this.alwaysCount--;

			if (this.slogs.length == 0)
				break;

			//
			// シーケンスログアイテムキューから、シーケンス番号が一致するアイテムを全て削除する
			//
			var delItem = this.slogs[this.slogs.length - 1];
			var type = SL_MESSAGE;

			while (delItem != null && delItem.seqNo == slog.seqNo)
			{
				type = delItem.type;
				this.slogs.length--;
				
				delItem = this.slogs[this.slogs.length - 1];
			}

			if (type == SL_IN)
				return;
		}
		while (false);

//		for (i in this.slogs)
		for (i = 0; i < this.slogs.length; i++)
			this.logBuffer[this.logBuffer.length] = this.slogs[i].str;

		this.slogs.length = 0;

		this.logBuffer[this.logBuffer.length] = str;

		if (slogCtrl.updateInterval == 0)
			this.update();
	},

	save: function()
	{
		slogCtrl.update();

		var content = document.getElementById('slogViewContent');
		var body =    document.getElementsByTagName('body').item(0);

		document.title = '';
		body.style.color = '#FFFFFF';
		body.style.backgroundColor = '#000000';
		body.innerHTML = content.innerHTML;
	},

	clear: function()
	{
		var content = document.getElementById('slogViewContent');
		content.innerHTML = '';
		slogCtrl.logBuffer.length = 0;
	},

	auto: function()
	{
		slogCtrl.updateInterval = 0;
		slogCtrl.update();

		var auto = document.getElementById('slogViewAuto');
		var stop = document.getElementById('slogViewStop');

		if (auto)
		{
			auto.style.display = 'none';
			stop.style.display = 'inline';
		}
	},

	stop: function()
	{
		slogCtrl.updateInterval = -1;

		var auto = document.getElementById('slogViewAuto');
		var stop = document.getElementById('slogViewStop');

		if (auto)
		{
			auto.style.display = 'inline';
			stop.style.display = 'none';
		}
	},
	
	update: function()
	{
		var content = document.getElementById('slogViewContent');
		var work ='';

		if (content)
		{
//			for (i in slogCtrl.logBuffer)
			for (i = 0; i < slogCtrl.logBuffer.length; i++)
			{
				work += slogCtrl.logBuffer[i] + '<br />';
			}
			
			slogCtrl.logBuffer.length = 0;
			content.innerHTML += work;
			content.scrollTop = content.scrollHeight;
		}
	},
	
	onmousedown: function(e)
	{
		if (window.createPopup)
			e = event;

		var x = e.clientX;
		var y = e.clientY;

//		if (navigator.appVersion.indexOf('Android') > 0)
//		{
//			e.preventDefault();
//			x = e.touches[0].clientX;
//			y = e.touches[0].clientY;
//		}

		slogCtrl.dragObj = document.getElementById('slogView');
		slogCtrl.beginX = x;
		slogCtrl.beginY = y;
		slogCtrl.beginLeft = slogCtrl.dragObj.offsetLeft;
		slogCtrl.beginTop =  slogCtrl.dragObj.offsetTop;

		e.cancelBubble = true;
		return false;
	},

	onmousemove: function(e)
	{
		if (slogCtrl.dragObj == null)
			return;

		if (window.createPopup)
			e = event;

		var x = e.clientX;
		var y = e.clientY;

//		if (navigator.appVersion.indexOf('Android') > 0)
//		{
//			e.preventDefault();
//			x = e.touches[0].clientX;
//			y = e.touches[0].clientY;
//		}

		var newLeft = slogCtrl.beginLeft + (x - slogCtrl.beginX);
		var newTop =  slogCtrl.beginTop  + (y - slogCtrl.beginY);

		var width =  slogCtrl.getWindowWidth();
		var height = slogCtrl.getWindowHeight();

		if (newLeft < 10)
			newLeft = 10;

		if (newLeft > width  - slogCtrl.dragObj.offsetWidth  - 10)
			newLeft = width  - slogCtrl.dragObj.offsetWidth  - 10;

		if (newTop < 10)
			newTop = 10;

		if (newTop >  height - slogCtrl.dragObj.offsetHeight - 10)
			newTop =  height - slogCtrl.dragObj.offsetHeight - 10;

		slogCtrl.dragObj.style.left = newLeft + 'px';
		slogCtrl.dragObj.style.top =  newTop +  'px';

		e.cancelBubble = true;
		return false;
	},
	
	onmouseup: function()
	{
		slogCtrl.dragObj = null;
	},

	getWindowWidth: function()
	{
		if (typeof window.innerWidth != 'undefined')
		    return window.innerWidth;

		if (document.compatMode == 'CSS1Compat')
		    return document.documentElement.clientWidth;

		return document.body.clientWidth;
	},

	getWindowHeight: function()
	{
		if (typeof window.innerHeight != 'undefined')
		    return window.innerHeight;

		if (document.compatMode == 'CSS1Compat')
		    return document.documentElement.clientHeight;

		return document.body.clientHeight;
	}
}

// === sequence log item ===
function SlogItem(seqNo, type, str)
{
	this.seqNo = seqNo;
	this.type = type;
	this.str = str;

	return this;
}

// === sequence log ===
function Slog(className, funcName, outputFlag)
{
	this.outputFlag = outputFlag || SL_KEEP;
	this.seqNo = slogCtrl.seqNo;
	slogCtrl.seqNo += 1;

	if (this.outputFlag == SL_ROOT)
		this.outputFlag = slogCtrl.rootFlag;

	var type = SL_IN;
	var str = slogCtrl.createString(this.seqNo, type, className, funcName);

	if (slogCtrl.alwaysCount ||
		this.outputFlag == SL_OUTPUT_ALL ||
		this.outputFlag == SL_ALWAYS)
	{
		slogCtrl.output(this, type, str);

		if (this.outputFlag == SL_ALWAYS)
			slogCtrl.alwaysCount++;
	}
	else
	{
		var item = new SlogItem(this.seqNo, type, str);
		slogCtrl.slogs[slogCtrl.slogs.length] = item;
	}

	return this;
}

Slog.prototype =
{
	stepOut : function()
	{
		var type = SL_OUT;
		var str = slogCtrl.createString(this.seqNo, type);

		slogCtrl.output(this, type, str);
	},

	message : function(level, str)
	{
		var type = SL_MESSAGE;
		var str = slogCtrl.createString(this.seqNo, type, level, str);

		if (level != SL_DEBUG)
		{
			slogCtrl.output(this, type, str);
		}
		else
		{
			var item = new SlogItem(this.seqNo, type, str);
			slogCtrl.slogs[slogCtrl.slogs.length] = item;
		}
	},

	d : function(str)
	{
		this.message(SL_DEBUG, str);
	},

	i : function(str)
	{
		this.message(SL_INFO, str);
	},

	w : function(str)
	{
		this.message(SL_WARN, str);
	},

	e : function(str)
	{
		this.message(SL_ERROR, str);
	}
}

// === begin ===
var slogCtrl = new SlogController();

if (window.addEventListener)
{
	window.addEventListener('load',       slogCtrl.init,        false);
//	window.addEventListener('mousedown',  slogCtrl.onmousedown, false);
//	window.addEventListener('mousemove',  slogCtrl.onmousemove, false);
//	window.addEventListener('mouseup',    slogCtrl.onmouseup,   false);

//	window.addEventListener('touchstart', slogCtrl.onmousedown, false);
//	window.addEventListener('touchmove',  slogCtrl.onmousemove, false);
//	window.addEventListener('touchend',   slogCtrl.onmouseup,   false);
}
else if (window.attachEvent)
{
	window.attachEvent('onload',      slogCtrl.init);
//	window.attachEvent('onmousedown', slogCtrl.onmousedown);
//	window.attachEvent('onmousemove', slogCtrl.onmousemove);
//	window.attachEvent('onmouseup',   slogCtrl.onmouseup);
}

window.document.onmousemove = slogCtrl.onmousemove;
window.document.onmouseup =   slogCtrl.onmouseup;
