
// 公共变量的定义 
var bodyHeight; 
var maxMsgCount = 10000;
var count = 0;
var picArray = new Array("doc","docx","xls","xlsx","ppt","pptx","gif","jpeg","jpg","png","bmp","pdf","swf","zip","rar","txt","avi","wma","rmvb","rm","mp4","3gp");
var $fdiv;
var amrplayerHandleId = 0;
var avatarStamp = '';
var msgContentScrollTop = -3;

var kAttachAutoDownloadType = "auto-download";
var kAttachAutoDisplayType  = "auto-display";
var kAttachDirType          = "dir";

var kAttachSuccessful       = "successful";
var kAttachCancel           = "cancel";
var kAttachError            = "error";              

//禁止后退键 作用于Firefox、Opera  
document.onkeypress=banBackSpace;
//禁止后退键  作用于IE、Chrome  
document.onkeydown=banBackSpace;

//处理键盘事件 禁止后退键（Backspace）密码或单行、多行文本框除外  
function banBackSpace(e){
	var ev = e || window.event;//获取event对象     
	var obj = ev.target || ev.srcElement;//获取事件源     

	var t = obj.type || obj.getAttribute('type');//获取事件源类型    

	//获取作为判断条件的事件类型  
	var vReadOnly = obj.getAttribute('readonly');
	var vEnabled = obj.getAttribute('enabled');
	//处理null值情况  
	vReadOnly = (vReadOnly == null) ? false : vReadOnly;
	vEnabled = (vEnabled == null) ? true : vEnabled;

	//当敲Backspace键时，事件源类型为密码或单行、多行文本的，  
	//并且readonly属性为true或enabled属性为false的，则退格键失效  
	var flag1=(ev.keyCode == 8 && (t=="password" || t=="text" || t=="textarea") && (vReadOnly==true || vEnabled!=true))?true:false;

	//当敲Backspace键时，事件源类型非密码或单行、多行文本的，则退格键失效  
	var flag2=(ev.keyCode == 8 && t != "password" && t != "text" && t != "textarea")?true:false;

	//判断  
	if(flag2){
		return false;
	}
	if(flag1){
		return false;
	}
}

function setCurLanguage() {
	var langCode = Message4Js.curLanguage();
	var langSrc = "qrc:/html/js/msglang_EN.js";
	if (langCode == "CN") {
		langSrc = "qrc:/html/js/msglang_CN.js";
	}
	var head= document.getElementsByTagName('head')[0]; 
    var script= document.createElement('script'); 
    script.type= 'text/javascript'; 
    script.src= langSrc; 
    head.appendChild(script);
}

$(document).ready(function () {
	
	try {

		setCurLanguage();

		count = $(".chatBox_msgList > div").size();
		maxMsgCount = Message4Js.maxMsgCount;

		var avatarDate = new Date();
		avatarStamp = avatarDate.getTime();

		// 消息显示
		Message4Js.displaymsg.connect(displaymsg);
		Message4Js.displaymsgs.connect(displaymsgs);
		Message4Js.displayTipMsg.connect(displayTipMsg);
		Message4Js.displaymsgAtTop.connect(displaymsgAtTop);
		Message4Js.displayTipMsgAtTop.connect(displayTipMsgAtTop);
		Message4Js.displayHistorySep.connect(displayHistorySep);
		Message4Js.displayHistorySepAtTop.connect(displayHistorySepAtTop);

		// 音频相关
		Message4Js.onAutoDownloadFinish.connect(onAutoDownloadFinish);
		Message4Js.onAutoDownloadError.connect(onAutoDownloadError);
		AmrPlayer.stopped.connect(onAmrPlayerStopped);
		AmrPlayer.error.connect(onAmrPlayerError);

		// 图片相关
		Message4Js.onAutoDisplayFinish.connect(onAutoDisplayFinish);
		Message4Js.onAutoDisplayError.connect(onAutoDisplayError);
		
		// 附件相关
		Message4Js.onFinish.connect(onFinish);
		Message4Js.onError.connect(onError);
		Message4Js.onAttachUploadFinish.connect(onAttachUploadFinish);
		Message4Js.onDirUploadFinished.connect(onDirUploadFinished);
		Message4Js.onDirDownloadFinished.connect(onDirDownloadFinished);

		// 附件传输中止
		Message4Js.onStopped.connect(onStopped);

		// 附件名称修改
		Message4Js.onDownloadChanged.connect(onDownloadChanged);

		// 下载进度条
		Message4Js.onProgress.connect(onProgress);

		// 清屏
		Message4Js.cleanup.connect(cleanup);

		// 获取历史消息相关
		Message4Js.showMoreMsgTip.connect(showMoreMsgTip);
		Message4Js.closeMoreMsgTip.connect(closeMoreMsgTip);
		Message4Js.showMoreMsgFinish.connect(showMoreMsgFinish);
		Message4Js.showMoreHistoryMsgTip.connect(showMoreHistoryMsgTip);
		
		// 消息发送失败
		Message4Js.messageSendError.connect(messageSendError);
		Message4Js.messageSent.connect(messageSent);
		
		// 密信相关
		Message4Js.setSendSecretRead.connect(setSendSecretRead);
		Message4Js.onRecvSecretRead.connect(onRecvSecretRead);

		// 消息撤回
		Message4Js.messageWithdrawed.connect(messageWithdrawed);

		// 名片变化
		Message4Js.updateChatName.connect(updateChatName);

		// UI完成
		Message4Js.initUIComplete.connect(initUIComplete);

	} catch(e) {
		alert("load exception" + e);
	}	

	$('#moreBarContext').bind("click", showMoreMsg);
});

// msgContent滚动函数
function onMsgContentScrolled(event) {
	
	event = event || window.event;
	var direction = event.wheelDelta && (event.wheelDelta > 0 ? "mouseup" : "mousedown");
	
	if (direction == "mouseup" && event.wheelDelta >= 120 && msgContentScrollTop == 0) {
		
		// 如果已经在最顶上，而且还在往上滚动，则获取更多消息
		var moreBarDisplay = $("#moreBar").css("display");
		if (moreBarDisplay != "block") {
			return;
		}
		
		if ($("#moreTipImg").attr("src") != "qrc:/html/images/more_msg.png") {
			return;
		}

		// 置为不可使用
		msgContentScrollTop = -4;
		
		showMoreMsg();
		
	} else if (direction == "mousedown" && msgContentScrollTop >= -3) {

		// 向下滚动恢复初始值
		msgContentScrollTop = -3;

	} else if (direction == "mouseup" && msgContentScrollTop >= -3) {
		
		// 更新消息位置
		var newTop = document.getElementById('msgContent').scrollTop;
		if (newTop == 0) {
			++msgContentScrollTop;
			if (msgContentScrollTop > 0) {
				msgContentScrollTop = 0;
			}
		} else {
			msgContentScrollTop = -3;
		}
	}

    // Message4Js.jsdebug("--------------------------------- " + msgContentScrollTop + " " + direction + " delta: " + event.wheelDelta);
}

// 页面加载
function onload(){

	$("#moreBarContext").html("<img id=\"moreTipImg\" src=\"qrc:/html/images/more_msg.png\" style=\"margin-bottom:-3px\">"+LANG_DICT["SPACE_CHECK_MORE_MESSAGE"]+"</div>");	

	onresize();
	
	Message4Js.loadFinished();
	
	// 绑定消息滚轮滚动事件
	document.getElementById('msgContent').onmousewheel = onMsgContentScrolled;
}

// 调整页面大小
function onresize(){

	var scrollHeight = document.getElementById('msgContent').scrollHeight;
	var scrollTop = document.getElementById('msgContent').scrollTop;
	var clientHeight = document.getElementById('msgContent').clientHeight;
	
	bodyHeight = document.documentElement.clientHeight;
	$("#msgContent").height(bodyHeight);
	
	// 滚动条控制
	if (scrollTop+clientHeight >= scrollHeight) {
		$("#msgContent").scrollTop(document.getElementById('msgContent').scrollHeight);
	}
	
	// 更新消息位置
	msgContentScrollTop = -3;
}

/*********************************** 消息显示 **************************************/
// 移除最顶层的一个消息
function RemoveFirst() {
	$(".chatBox_msgList div:first").remove();
	count = count - 1;
}

// 创建消息时间
function createMsgTime(obj, atTop) {

	if (obj === null) {
		return null;
	}

	var time;
	if (typeof obj.time !== "string" || obj.time == null) {
		var d = new Date();
		time = d.getFullYear() + "-" + (d.getMonth() + 1) + "-" + d.getDate() + " " + d.toLocaleTimeString();
	} else {
		time = obj.time;
	}

	var timeStr = Message4Js.displayTime(time);

	var display = 'block';
	var msgTimes = $(".msgtime");
	if (msgTimes.size() > 0) {
		if (atTop) {
			var firstMsgTime = msgTimes.first();
			var firstTime = firstMsgTime.attr('msg-time');
			if (Message4Js.withinMinutes(time, firstTime, 1)) {
				firstMsgTime.css('display', 'none');
			}
		} else {
			var lastTime = msgTimes.last().attr('msg-time');
			var lastShowTime = '';
			var visibleMsgTimes = msgTimes.filter(":visible");
			if (visibleMsgTimes.size() > 0) {
				lastShowTime = visibleMsgTimes.last().attr('msg-time');
			}
			if (lastShowTime.length > 0) {
				if (Message4Js.withinMinutes(lastTime, time, 1) && Message4Js.withinMinutes(lastShowTime, time, 2)) {
					display = 'none';
				}
			}
		}
	}

	var ret = '';
	ret += '	<div class="msgtime" msg-time="' + time + '" style="display:' + display + ';">';
	ret += '		<a href="javascript:void(0)">';
	ret += '			<span>' + timeStr + '</span>';
	ret += '		</a>';
	ret += '	</div>';

	return ret;
}


// 创建消息标题
function createTitle(obj) {
	
	if (obj === null) {
		return null;
	}

	var ret = $("<div/>").addClass("msgHead");
	var span = $("<span/>");
	
	var content;
	if (obj.method === null || ( typeof obj.method === "string" && obj.method === "Method_Send")) {
		content = (Message4Js.uname === null || Message4Js.uname == "" ? Message4Js.uid : Message4Js.uname);
	} else {
		content = (obj.uname === null||obj.uname == "" ? obj.uid : obj.uname);

		if (obj.gid !== null && typeof obj.gid === "string" && obj.gid !== "") {
			if (obj.uid !== Message4Js.uid && obj.uid !== null) {
				// group receive
				span.addClass('cardname').attr('msgtype', obj.msgtype).attr('gid', obj.gid).attr('uid', obj.uid);
			}
		}
	}
	
	span.text(content);

	ret.append(span);

	return ret;
}

// 获取图片大小
function getAutodisplaySizeByUrl(url, widthHint, heightHint) {
	var obj = Message4Js.getAutodisplaySizeByUrl(url, widthHint, heightHint);
	return obj;
}

// 对自动播放图片的处理
function dealAutoDisplay(context,obj){
	
	var direct;

	// 发送
	if ((obj.method === null || ( typeof obj.method === "string" && obj.method === "Method_Send")) && obj.sync != 1) {
		direct = "send"
	// 接收
	} else {
		direct = "receive"
	}

	var secretFlag = '0';
	if (isMsgSecret(obj)) {
		secretFlag = '1';
	}
	
	// 附件的处理
	$.each(obj.attachs, function(n,value) {
	
		// 自动显示处理(图片)
		if(value.type != undefined && value.type == kAttachAutoDisplayType){
			
			if(direct == 'send'){ 
				var path = ((value.path != undefined) ? value.path : "");
				var size = getAutodisplaySizeByUrl(path, value.picwidth, value.picheight);
				var width = size.width;
				var height = size.height;
				if (width > 0 && height > 0) {
					var str = "<div uuid='"+value.uuid+"_autodisplay' secret=\""+secretFlag+"\">";
					str +="<image class='img_autodisplay' title='"+LANG_DICT["DOUBLE_CLICK_VIEW_IMAGE"]+"' origsrc=\""+value.path+"\" src=\""+Message4Js.getAutodisplayImgSrc(value.path)+"\" width=\""+width+"\" height=\""+height+"\" secret=\""+secretFlag+"\" ondblclick=\"openImages('"+value.path+"')\" />";
					str +="</div>";
					context = context.replaceAll("{"+value.uuid+"}",str);
				} else {
					var str = "<div uuid='"+value.uuid+"_autodisplay' secret=\""+secretFlag+"\">";
					str +="<image src=\"qrc:/images/errorBmp.png\" alt=\""+LANG_DICT["IMAGE_NOT_EXIST"]+"\""
					str +=" />";
					str +="</div>";
					context = context.replaceAll("{"+value.uuid+"}",str);
				}
			}else{
				var imageOK = false;

				if (value.result != undefined && value.result == kAttachSuccessful) {
					var path = ((value.path != undefined) ? value.path : "");
				 	var size = getAutodisplaySizeByUrl(path, value.picwidth, value.picheight);
					var width = size.width;
					var height = size.height;
					// 图片下载成功，并且本地有图片，则显示图片
					if (width > 0 && height > 0) {
						var str = "<div uuid='"+value.uuid+"_autodisplay' secret=\""+secretFlag+"\">";
						str +="<image class='img_autodisplay' title='"+LANG_DICT["DOUBLE_CLICK_VIEW_IMAGE"]+"' uuid='img_"+value.uuid+"_autodisplay' origsrc=\""+value.path+"\" src=\""+Message4Js.getAutodisplayImgSrc(value.path)+"\" width=\""+width+"\" height=\""+height+"\" secret=\""+secretFlag+"\" ondblclick=\"openImages('"+value.path+"')\" />";
						str +="</div>";
						context = context.replaceAll("{"+value.uuid+"}",str);

						imageOK = true;
					}
				} 

				// 如果图片没有下载完成，则显示正载下载
				if (!imageOK) {
					var str = "<div uuid='"+value.uuid+"_autodisplay' secret=\""+secretFlag+"\">";
					str +="<image src=\"qrc:/images/sendingPic.gif\" alt=\""+LANG_DICT["DOWNLOADING_IMAGE"]+"\" />";
					str +="</div>";
					context = context.replaceAll("{"+value.uuid+"}",str);
				}
			}
		}	
	});
	
	return context;
}

// 创建消息内容
function createMsgBody(obj, isSend) {
	
	if (obj === null) {
		return null;
	}

	var ret = "";

	if (typeof obj.body == "string" && obj.body != "") {

		if (isSend) {
			if (typeof obj.sequence == "string" && obj.sequence != "") {
				if (isMsgSecret(obj)) {
					ret = $("<div/>").addClass("msgBody").attr("id", obj.sequence + "_msg_body").attr("uid", obj.uid)
					.attr('body-stamp', obj.stamp).attr('secret-type', 'text').css("display", "none");
					if (isMsgRead(obj)) {
						return ret;
					}
				} else {
					ret = $("<div/>").addClass("msgBody").attr("id", obj.sequence + "_msg_body");
				}		
			} else {
				ret = $("<div/>").addClass("msgBody");
			}
		} else {
			if (typeof obj.stamp == "string" && obj.stamp != "") {
				if (isMsgSecret(obj)) {
					ret = $("<div/>").addClass("msgBody").attr("id", obj.stamp + "_msg_body").attr("uid", obj.uid)
					.css("display", "none");
					if (isMsgRead(obj)) {
						return ret;
					}
				} else {
					ret = $("<div/>").addClass("msgBody").attr("id", obj.stamp + "_msg_body");
				}		
			} else {
				ret = $("<div/>").addClass("msgBody");
			}
		}

		var context = obj.body;
		var context = Message4Js.parseFaceSymbol(context);

		// 对自动播放图片的处理
		context = dealAutoDisplay(context, obj);

		// 处理超链接
		context = replaceurl(context);

		if (obj.ext !== null)
		{
			if ((typeof obj.ext.type == "string" && obj.ext.type == "at") &&
			    (typeof obj.ext.at == "string" && obj.ext.at != "") &&
			    (typeof obj.ext.atid == "string" && obj.ext.atid != "")) {
			    context = Message4Js.parseAtSymbol(context, obj.ext.at, obj.ext.atid);
			}
			    
			if (typeof obj.ext.type == "string" && obj.ext.type == "share" &&
			 typeof obj.ext.shareurl == "string" && obj.ext.shareurl != "") {
				context += "<br/>"+LANG_DICT["SHARE_FROM_SPACE"];
				context += "<a href=\"#\" onclick=\"openLinkUrl(\'" +obj.ext.shareurl+ "\')\">" +obj.ext.shareurl+ "</a>";
			}
		}
		
		// 加入发送失败标记
		if (typeof obj.sequence == "string" && obj.sequence != "") {
			ret.append("<div id=\"" + obj.sequence + "_send_error_tip\"" + " class=\"sendErrorTip\"><img src='qrc:/html/images/tip_warn.png'/></div>");
		}
	
		ret.append("<div class=\"bodyContext\">"+context+"</div>");
	}

	return ret;
}


// 上传文件的类型图片库中是否存在
function isConFormat(format){
	for(var z=0;z<picArray.length;z++){
		if(picArray[z] == format){
			return true;
		}
	}
	return false;
}

// 附件大小
function getAttachSizeString(fileLength) {
	var fileLengthStr = "";
	if(fileLength < 1024){
		fileLengthStr = fileLength+" B";
	}else if(fileLength < 1024*1024){
		fileLengthStr = (fileLength/1024).toFixed(2)+" K";
	}else if(fileLength < 1024*1024*1024) {
		fileLengthStr = (fileLength/(1024*1024)).toFixed(2)+" M";
	}else {
		fileLengthStr = (fileLength/(1024*1024*1024)).toFixed(2)+" G";
	}
	return fileLengthStr;
}

// 附件文件名
function getAttachShowName(fileName) {
	var maxCharLength = 300; // 15个中文字或者20个英文字母
	var charLength = 0;
	var i;
	for (i = 0; i < fileName.length; i++) {
		if (fileName.charCodeAt(i)>256) {
			charLength += 20;
		} else {
			charLength += 15;
		}

		if (charLength >= maxCharLength)
			break;
	}

	var showName;
	if (i+1 < fileName.length)
		showName = fileName.substr(0, i+1) + "...";
	else
		showName = fileName;

	return showName;
}

// 获取附件信息
function getAttachments(obj,direct){
	
	var attachments ="";

	$.each(obj.attachs, function(n,value) {
	
		if(value.type != undefined && value.type == kAttachAutoDisplayType){	

		} else if(value.type != undefined && value.type == kAttachAutoDownloadType){	

			if (direct == 'receive') { // audio received
				var autoDestroy = "1";
				if (obj.sync == 1) {
					autoDestroy = "0";
				}
				if (value.result != kAttachSuccessful) {
					var str = "<div class=\"autodownload\" id=\""+value.uuid+"_autodownload\" play-time=\""+value.time+"\" style=\"height:35px;\" file-src=\""+value.path+"\">";
					str +="<span style=\"vertical-align: middle; color:#666;\" >"+LANG_DICT["VOICE_MESSAGE_SPACE"]+"</span><image style=\"vertical-align: middle;\" src=\"qrc:/html/images/loading.gif\" alt=\""+LANG_DICT["DOWNLOADING_VOICE"]+"\" />";
					str +="</div>";
					if (isMsgSecret(obj)) {
						str += "<div class = \"audioplaysecret\" id = \""+value.uuid+"_audioplaysecret\" body-stamp = \""+obj.stamp+"\" auto-destroy = \""+autoDestroy+"\" read = \"0\" read-time = \"10\"" + 
						       ((obj.sync == 1) ? ("audioplay-seq = \""+obj.sequence+"\"") : "") +
						       ">" +
							   "<img id = \""+value.uuid+"_audioplaylock\" src='qrc:/html/images/lock.png'/>" + 
							   "</div>";
					}
					attachments = attachments + str;
				} else {
					var str = "<div class=\"autodownload\" id=\""+value.uuid+"_autodownload\" play-time=\""+value.time+"\" style=\"height:35px;\" file-src=\""+value.path+"\">";
					str += "<a class = \"audioplay\" onclick=\"startplayAmr('" + value.uuid + "')\"></a>";
					str += "<div class = \"audioplayinfo\">" + value.time+　"″</div>";
					str +="</div>";
					if (isMsgSecret(obj)) {
						str += "<div class = \"audioplaysecret\" id = \""+value.uuid+"_audioplaysecret\" body-stamp = \""+obj.stamp+"\" auto-destroy = \""+autoDestroy+"\" read = \"0\" read-time = \"10\"" +
						       ((obj.sync == 1) ? ("audioplay-seq = \""+obj.sequence+"\"") : "") +
						       ">" + 
							   "<img id = \""+value.uuid+"_audioplaylock\" src='qrc:/html/images/lock.png'/>" + 
							   "</div>";
					}
					attachments = attachments + str;
				}
			} else { // audio send
				var str = "<div class=\"autodownload\" id=\""+value.uuid+"_autodownload\" play-time=\""+value.time+"\" style=\"height:35px;\" file-src=\""+value.path+"\">";
				var audiostr = "<a class = \"audioplay\" onclick=\"startplayAmr('" + value.uuid + "')\"></a>";
				audiostr += "<div class = \"audioplayinfo\">" + value.time + "″</div>";
				str += audiostr; 
				str +="</div>";
				if (isMsgSecret(obj)) {
					str += "<div class = \"audioplaysecret\" id = \""+value.uuid+"_audioplaysecret\" auto-destroy = \"0\" audioplay-seq = \""+obj.sequence+"\">" + 
						   "<img id = \""+value.uuid+"_audioplaylock\" src='qrc:/html/images/lock.png'/>" + 
						   "</div>";
				}
				attachments = attachments + str;
			}

		} else {

			// 文件大小显示处理
			var fileLength = value.size;
			var fileLengthStr = getAttachSizeString(fileLength);
			
			// 文件名称长度处理
			var fileNameStr = getAttachShowName(value.savedname);
			
			var format = value.format;
			var formatStr = format;
			if(!isConFormat(format)){
				formatStr = "other";
			}
			
			var tipText = "";
			if(direct == 'send'){
				if(value.result != undefined && value.result == kAttachSuccessful){
					tipText = "";
				}else{
					tipText = "<a href=\"javascript:void(0);\" class=\"startupload\" onclick=\"startupload(\'"+value.uuid+"\')\"></a>";
				}
			}else{
				if(value.result != undefined && value.result == kAttachSuccessful){
					tipText = "";
				}else{
					tipText = "<a href=\"javascript:void(0);\" class=\"startdownload\" onclick=\"startdownload(\'"+value.uuid+"\')\"></a>";
				}
			}

			var str = " <div style='padding-top:7px; padding-right:10px; padding-bottom:7px; padding-left:10px;' class='attachContainer'>";
			str += "	<div id='fileFocue_"+value.uuid+"' class='fileFocueOut'>";
			str += "		<div style='float:left;'>";
			if (value.type == kAttachDirType) {
				str += "		<img class=\"attachImage\" src='qrc:/html/images/file_formats/folder.png'/>";
			} else {
				str += "		<img class=\"attachImage\" src='qrc:/html/images/file_formats/"+formatStr+".png'/>";
			}
			str += "		</div>";
			str += "		<div class=\"attachInfoText\" style='float:left; margin-left: 10px;'>";
			str += "			<span class=\"attachNameText\" style='color:#666666;' id=\""+value.uuid+"_attname\" title=\""+value.savedname+"\">"+fileNameStr+"</span>";
			str += "			<br/>";
			str += " 			<span class=\"attachSizeText\" style='color:#a0a0a0;'>"+fileLengthStr+"&nbsp;&nbsp;</span>";
			str += "		</div>";
			str += "	</div>";
			str += "	<div id=\""+value.uuid+"_process\" style=\"min-height: 18px;\" class=\"process\">";
			if(value.result != undefined && value.result != kAttachSuccessful){
				str += "		<div class=\"progress\">";
				str += "			<span id = \""+ value.uuid +"_progresswidth\" class=\"blue\" style=\"width: 0%;\"><span></span></span>";
				str += "		</div>";
				str += "		<div id = \""+ value.uuid +"_attacheoperatortip\" class=\"attacheoperatortip\">";
				str += "		  &bnsp;";
				str += "		</div>";
				str += "		<div id=\"" + value.uuid + "_tipText\" class=\"attacheoperator\">";
				str += 				tipText;
				str += "		</div>"
			}else if(value.result != undefined && value.result == kAttachSuccessful){
				if (value.type == kAttachDirType) {
					str += " 		<a href=\"#\" onclick=\"openFile(\'"+value.uuid+"\')\">"+LANG_DICT["OPEN"]+"</a>&nbsp;&nbsp;\
						<a href=\"#\" onclick=\"openFileDir(\'"+value.uuid+"\');\">"+LANG_DICT["OPEN_DIR"]+"</a>";
					// if (direct == 'receive') {
						str += "&nbsp;&nbsp;<a href=\"#\" onclick=\"copyDir(\'"+value.uuid+"\');\">"+LANG_DICT["COPY"]+"</a>";
					// }
				} else {
					if(direct == 'send'){
						str += " 		<a href=\"#\" onclick=\"openFile(\'"+value.uuid+"\')\">"+LANG_DICT["OPEN_FILE"]+"</a>&nbsp;&nbsp;\
						<a href=\"#\" onclick=\"openFileDir(\'"+value.uuid+"\');\">"+LANG_DICT["OPEN_DIR"]+"</a>";
					} else {
						str += " 		<a href=\"#\" onclick=\"openFile(\'"+value.uuid+"\')\">"+LANG_DICT["OPEN_FILE"]+"</a>&nbsp;&nbsp;\
						<a href=\"#\" onclick=\"openFileDir(\'"+value.uuid+"\');\">"+LANG_DICT["OPEN_DIR"]+"</a>&nbsp;&nbsp;\
						<a href=\"#\" onclick=\"fileSaveAs(\'"+value.uuid+"\');\">"+LANG_DICT["SAVE_AS"]+"</a>";
					}
				}
			}
			str += "	</div>";
			str += "</div>";
			
			attachments = attachments + str;
		}
	});
	
	return attachments;
}

// 创建消息附件
function createAttachment(obj){
	
	if (obj === null) {
		return null;
	}
	
	var attach = "";
	if ((obj.method === null || ( typeof obj.method === "string" && obj.method === "Method_Send")) && obj.sync != 1) { // 发送
		attach = getAttachments(obj,"send");
	} else { // 接收
		attach = getAttachments(obj,"receive");
	}

	var ret = "";
	if (attach != "") {
		if (obj.method === null || ( typeof obj.method === "string" && obj.method === "Method_Send")) { // 发送

			if (typeof obj.sequence == "string" && obj.sequence != "") {
				ret = $("<div/>").addClass("msgBody").attr("id", obj.sequence + "_msg_body").attr("uid", obj.uid)
					.attr('body-stamp', obj.stamp).attr('secret-type', 'voice');
				var read = isMsgRead(obj);
				if (!read) {
					// 加入发送失败标记
					ret.append("<div id=\"" + obj.sequence + "_send_error_tip\"" + " class=\"sendErrorTip\"><img src='qrc:/html/images/tip_warn.png'/></div>");
					ret.append(attach);
				}
				
			} else {
				ret = $("<div/>").addClass("msgBody");
				ret.append(attach);
			}

		} else { // 接收

			if (typeof obj.stamp == "string" && obj.stamp != "") {
				ret = $("<div/>").addClass("msgBody").attr("id", obj.stamp + "_msg_body").attr("uid", obj.uid);
				var read = isMsgRead(obj);
				if (!read) {
					ret.append(attach);
				}		
			} else {
				ret = $("<div/>").addClass("msgBody");
				ret.append(attach);
			}
		}		
	}
	
	return ret;
}


// 附件自动上传
function startAutoUpload(uuid) {
	Message4Js.startAutoUpload(uuid);
}

// 附件上传
function startupload(uuid){
	if (!Message4Js.startupload(uuid))
		return;
		
	$("#"+ uuid +"_attacheoperatortip").css("display","none");
	var tipText = "<a href=\"javascript:void(0);\" class=\"stopupload\" onclick=\"stopupload('" + uuid + "')\"></a>";
	$("#"+uuid+"_tipText").html(tipText);	
}

// 附件自动下载
function startAutoDownload(uuid){
	Message4Js.startAutoDownload(uuid);
}

// 组装消息
function makeMessage(obj, atTop) {
	// 消息
	var msg = $("<div/>");
	
	var msgType;
	var origin;
	var avatarUid = '';
	if (obj.method === null || ( typeof obj.method === "string" && obj.method === "Method_Send")) {
		msgType = "owner"; 
		origin = "send";
		msg.addClass("chatBox_myMsg");
		avatarUid = Message4Js.uid;
	} else {
		msgType = "other"; 
		if (obj.uid == Message4Js.uid && obj.uid !== null) {
			origin = "send";
			msg.addClass("chatBox_myMsg");
			avatarUid = Message4Js.uid;
		} else {
			origin = "receive";
			msg.addClass("chatBox_buddyMsg");
			avatarUid = obj.uid;
		}
	}

	var groupReceive = false;
	var chatType;
	if (obj.gid !== null && typeof obj.gid === "string" && obj.gid !== "") {
		if (obj.uid !== Message4Js.uid && obj.uid !== null) {
			groupReceive = true;			
		}
		chatType = 'group';
	} else {
		chatType = 'chat';
	}

	// @消息的anchor
	if (obj.ext !== null)
	{
		if ((typeof obj.ext.type == "string" && obj.ext.type == "at") &&
			(typeof obj.ext.at == "string" && obj.ext.at != "") &&
			(typeof obj.ext.atid == "string" && obj.ext.atid != "")) {
			var atAnchor = $("<a/>");
			atAnchor.attr("name", obj.ext.atid);
			msg.append(atAnchor);
		}
	}

	// 时间
	var time = createMsgTime(obj, atTop);
	if (time !== null) {
		msg.append(time);
	}

	var msgContainer = $("<div/>");
	msgContainer.addClass('msgContainer');
	msgContainer.addClass('msgContainer-'+origin);
	msgContainer.addClass('clearfix');

	// 头像
	var msgAvatar = '';
	msgAvatar += '<i class = "' + origin + '">';
	
	// 头像使用Qt的扩展实现
	var avatarGid = "";
	if (obj.gid !== null && typeof obj.gid === "string" && obj.gid !== "") {
		avatarGid = obj.gid;
	}
	msgAvatar += '	<object type="application/x-qt-plugin" classid="user_avatar" width="34" height="34" \
	                        other_id="'+obj.uid+'" chat_type="'+chatType+'" \
	                        user_id="'+avatarUid+'" group_id="'+avatarGid+'"/>';

	msgAvatar += '</i>';
	msgContainer.append(msgAvatar);

	// 箭头 
	if (obj.pure_image != 1) { // pure image do not need arrow
		var msgArrow = '';
		if (groupReceive) {
			msgArrow = '<div class="arrow-down arrow-down-'+ origin +'" style="top: 25px;"></div>';
		} else {
			msgArrow = '<div class="arrow-down arrow-down-'+ origin +'" style="top: 9px;"></div>';
		}
		msgContainer.append(msgArrow);
	}

	// 标题
	if (groupReceive) {
		var header = createTitle(obj);
		if (header !== null) {
			msgContainer.append(header);
		}
	}

	var msgBodyContainer = $("<div/>");
	msgBodyContainer.addClass('msgBodyContainer-'+origin);
	if (obj.pure_image == 1) {
		msgBodyContainer.addClass('nonBubble-'+origin);
	} else {
		msgBodyContainer.addClass('bubble-'+origin);
	}
	if (origin == "send") {
		msgBodyContainer.attr('sequence', obj.sequence);
	}
	if (typeof obj.stamp == "string" && obj.stamp != "") {
		msgBodyContainer.attr('stamp', obj.stamp);
	}
	if (isMsgSecret(obj)) {
		msgBodyContainer.attr('secret', '1');
	} else {
		msgBodyContainer.attr('secret', '0');
	}
	
	// 密信查看
	var secret = null;
	if (isMsgSecret(obj)) {
		if (msgType == "owner") {
			secret = createSendSecret(obj);
		} else {
			secret = createRecvSecret(obj);
		}
	}

	if (secret !== null) {
		msgBodyContainer.append(secret);
	}

	// 消息体
	var content = createMsgBody(obj, (msgType == "owner"));
	if (content !== null) {
		msgBodyContainer.append(content);
	}

	// 附件
	var attachment = createAttachment(obj);
	if (attachment !== null) {
		msgBodyContainer.append(attachment);
	}

	msgContainer.append(msgBodyContainer);
	msg.append(msgContainer);

	return msg;
}

// 消息的展示
function displaymsg(obj) {

	if (obj === null) {
		return false;
	}

	// 消息数量
	if (count >= maxMsgCount) {
		RemoveFirst();
	}

	// 消息
	var msg = makeMessage(obj, false);
	
	var scrollHeight = document.getElementById('msgContent').scrollHeight;
	var scrollTop = document.getElementById('msgContent').scrollTop;
	var clientHeight = document.getElementById('msgContent').clientHeight;

	$(".chatBox_msgList").append(msg);
	count++;
	
	if(obj.method === null || ( typeof obj.method === "string" && obj.method === "Method_Send")){					
		$("#msgContent").scrollTop(document.getElementById('msgContent').scrollHeight);
	}else{
		if(scrollTop+clientHeight >= scrollHeight){
			$("#msgContent").scrollTop(document.getElementById('msgContent').scrollHeight);
		}
	}

	if ((obj.method === null || ( typeof obj.method === "string" && obj.method === "Method_Send")) && obj.sync != 1) {
		$.each(obj.attachs, function(n,value) {
			if (value.result != kAttachSuccessful) {
				if(value.type != undefined && (value.type == kAttachAutoDisplayType || value.type == kAttachAutoDownloadType)){	
					startAutoUpload(value.uuid);
				} else {
					startupload(value.uuid);
				}
			}
		});
	} 
	else {
		// 自动下载图片或语音
		$.each(obj.attachs, function(n,value) {	
			if (value.result != kAttachSuccessful) {
				if(value.type != undefined && (value.type == kAttachAutoDisplayType || value.type == kAttachAutoDownloadType)){	
					startAutoDownload(value.uuid);
				}
			} else if (value.type == kAttachAutoDisplayType && value.result == kAttachSuccessful && !Message4Js.isImageOk(value.path)) {
				startAutoDownload(value.uuid);
			}
		});
	}

	return true;
}

// 消息的展示
function displaymsgAtTop(obj) {

	if (obj === null) {
		return false;
	}

	// 消息
	var msg = makeMessage(obj, true);
	
	$(".chatBox_msgList").prepend(msg);
	count++;

	if ((obj.method === null || ( typeof obj.method === "string" && obj.method === "Method_Send")) && obj.sync != 1) {
		$.each(obj.attachs, function(n,value) {
			if (value.result != kAttachSuccessful) {
				if(value.type != undefined && (value.type == kAttachAutoDisplayType || value.type == kAttachAutoDownloadType)){	
					startAutoUpload(value.uuid);
				} else {
					startupload(value.uuid);
				}
			}
		});
	} 
	else {
		// 自动下载图片或语音
		$.each(obj.attachs, function(n,value) {	
			if (value.result != kAttachSuccessful) {
				if(value.type != undefined && (value.type == kAttachAutoDisplayType || value.type == kAttachAutoDownloadType)){	
					startAutoDownload(value.uuid);
				}
			} else if (value.type == kAttachAutoDisplayType && value.result == kAttachSuccessful && !Message4Js.isImageOk(value.path)) {
				startAutoDownload(value.uuid);
			}
		});
	}

	return true;
}

// 消息组的展示
function displaymsgs(obj) {
	$.each(obj, function(n,value) {
		displaymsg(value);
	});
}

// 组装提示消息
function makeTipMessage(infomsg, level, action, param) {
	var msg = $("<div/>");
	msg.addClass("chatBox_infoMsg");
	
	var tipBody = $("<div/>").addClass("msgBody");
	var tipContainer = $("<div/>").addClass("tipContainer");
	
	// 提示图标
	var ret = "";
	var tipImage = $("<div/>").addClass("tipImage");
	if (level == "info") {
	    tipImage.html("<img src='qrc:/html/images/tip_info.png' />");
	}
	else if (level == "warn") {
		tipImage.html("<img src='qrc:/html/images/tip_warn.png' />");	
	}
	else if (level == "error") {
		tipImage.html("<img src='qrc:/html/images/tip_error.png' />");	
	}
	else {
		tipImage.html("<img src='qrc:/html/images/tip_ok.png' />");
	}
	
	var content = infomsg;
	
	// 链接处理
	if (action != null && action != "") {
		var tipAnchor = "<a href=\"#\" onclick=\"tipActionClicked(\'" +param+ "\', this)\">" +action+ "</a>";
		content += tipAnchor;
	}
	var tipContent = $("<div style = \"word-break:break-all;\"/>");
	tipContent.append(content);
	
	// 组装提示消息
	tipContainer.append(tipImage);
	tipContainer.append(tipContent);
	tipBody.append(tipContainer);
	msg.append(tipBody);

	return msg;
}

// 展示消息
function displayTipMsg(isSend, displaytime, infomsg, level, action, param, obj) {
	
	if (displaytime == null || infomsg == null || level == null) {
		return false;
	}

	// 消息数量
	if (count >= maxMsgCount) {
		RemoveFirst();
	}

	var msg = makeTipMessage(infomsg, level, action, param);	
	
	var scrollHeight = document.getElementById('msgContent').scrollHeight;
	var scrollTop = document.getElementById('msgContent').scrollTop;
	var clientHeight = document.getElementById('msgContent').clientHeight;
	
	$(".chatBox_msgList").append(msg);
	count++;

	if(!isSend){					
		if(scrollTop+clientHeight >= scrollHeight){
			$("#msgContent").scrollTop(document.getElementById('msgContent').scrollHeight);
		}
	}else{
		$("#msgContent").scrollTop(document.getElementById('msgContent').scrollHeight);
	}

	return true;
}

// 展示消息
function displayTipMsgAtTop(isSend, displaytime, infomsg, level, action, param, obj) {
	
	if (displaytime == null || infomsg == null || level == null) {
		return false;
	}

	var msg = makeTipMessage(infomsg, level, action, param);
	
	$(".chatBox_msgList").prepend(msg);
	count++;
	
	return true;
}

function makeHistorySep() {
	var msg = $("<div/>");
	msg.addClass("chatBox_historySep");
	msg.html("<img src='qrc:/html/images/history-sep_L.png' /><span style='color: #9f9f9f; position: relative; top: 3px;'>&nbsp;&nbsp;"+LANG_DICT["HISTORY_ABOVE"]+"&nbsp;&nbsp;</span><img src='qrc:/html/images/history-sep_R.png' />");
	return msg;
}

// 添加历史分隔条
function displayHistorySep() {
	var msg = makeHistorySep();
	
	var scrollHeight = document.getElementById('msgContent').scrollHeight;
	var scrollTop = document.getElementById('msgContent').scrollTop;
	var clientHeight = document.getElementById('msgContent').clientHeight;
	
	$(".chatBox_msgList").append(msg);

	if(scrollTop+clientHeight >= scrollHeight){
		$("#msgContent").scrollTop(document.getElementById('msgContent').scrollHeight);
	}
	
	return true;
}

// 添加历史分隔条
function displayHistorySepAtTop() {
	var msg = makeHistorySep();
	
	$(".chatBox_msgList").prepend(msg);
	
	return true;
}

// 消息发送失败
function messageSendError(seq) {
	$("#" + seq + "_send_error_tip").css("display","block");
}

// 消息发送成功
function messageSent(seq, stamp) {
	$("div.msgBodyContainer-send[sequence='"+seq+"']").attr('stamp', stamp);
	$("#" + seq + "_msg_body").attr('body-stamp', stamp);
	$("#" + seq + "_secret").attr('secret-stamp', stamp);
}

// 消息撤回成功
function messageWithdrawed(stamp, tipText) {
	var $sendMsg = $("div.msgBodyContainer-send[stamp='"+stamp+"']").closest('.chatBox_myMsg');
	if ($sendMsg.length > 0) {
		var infoMsg = makeTipMessage(tipText, "info", "", "");
		$sendMsg.replaceWith(infoMsg);
	}

	var $recvMsg = $("div.msgBodyContainer-receive[stamp='"+stamp+"']").closest('.chatBox_buddyMsg');
	if ($recvMsg.length > 0) {
		var infoMsg = makeTipMessage(tipText, "info", "", "");
		$recvMsg.replaceWith(infoMsg);
	}
}

// 名片变化更新名字
function updateChatName() {
	$('span.cardname').each(function(){
      var uid = $(this).attr('uid');
      if (uid !== null && uid !== "") {
      	var msgType = $(this).attr('msgtype');
      	var gid = $(this).attr('gid');
      	var chatName = Message4Js.chatName(msgType, gid, uid);
      	$(this).text(chatName);
      }
    });
}

// 消息是否是密信
function isMsgSecret(obj) {
	var secret = false;
	
	if (obj === null) {
		return secret;
	}
	
	if (typeof obj.ext.type == "string" && obj.ext.type == "secret") {
		secret = true;
	}
	
	return secret;
}

// 消息是否已读
function isMsgRead(obj) {
	var read = false;
	
	if (obj === null) {
		return read;
	}
	
	if (obj.readstate == 1) {
		read = true;
	}
	
	return read;
}

// 是否是语音消息
function isMsgVoice(obj) {
	var voice = false;
	
	if (obj === null) {
		return voice;
	}
	
	$.each(obj.attachs, function(n,value) {	
		if(value.type != undefined && value.type == kAttachAutoDownloadType) {	
			voice = true;
		}
	});
	
	return voice;
}

// 发送的密信
function createSendSecret(obj) {

	if (obj === null) {
		return null;
	}
	
	if (obj.sequence === "") {
		return null;
	}
	
	var voice = isMsgVoice(obj);
	
	var read = isMsgRead(obj);
	
	var ret = null;
	if (!voice) {
		ret = $("<div/>").addClass("secret").attr("id", obj.sequence + "_secret").attr('secret-stamp', obj.stamp);
	} else {
		if (!read) {
			ret = $("<div/>").addClass("secret").attr("id", obj.sequence + "_secret").attr('secret-stamp', obj.stamp).css("display", "none");
		} else {
			ret = $("<div/>").addClass("secret").attr("id", obj.sequence + "_secret").attr('secret-stamp', obj.stamp);
		}
	}
	
	if (!voice) {
		if (!read) {
			ret.append("<a href=\"#\" onclick=\"showSendBody('" + obj.sequence +"')\">"+LANG_DICT["CLICK_VIEW"]+"</a>");
			ret.append("<img src='qrc:/html/images/lock.png'/>");
		} else {
			ret.append("<img src='qrc:/html/images/text.png'/>");
			ret.append("<span>"+LANG_DICT["DESTROYED"]+"</span>");
		}
	} else {
		ret.append("<img src='qrc:/html/images/voice.png'/>");
		ret.append("<span>"+LANG_DICT["DESTROYED"]+"</span>");
	}

	return ret;
}

// 查看发送的密信
function showSendBody(seq) {
	var scrollHeight = document.getElementById('msgContent').scrollHeight;
	var scrollTop = document.getElementById('msgContent').scrollTop;
	var clientHeight = document.getElementById('msgContent').clientHeight;

	// 显示消息体，同时改为隐藏操作
	$("#" + seq + "_msg_body").css("display", "block");
	$("#" + seq + "_secret").html("<a href=\"#\" onclick=\"hideSendBody('" + seq +"')\">"+LANG_DICT["CLICK_HIDE"]+"</a>" + 
								  "<img src='qrc:/html/images/unlock.png'/>");

	// 如果在最下面则滚动到最下面
	if (scrollTop+clientHeight >= scrollHeight){
		$("#msgContent").scrollTop(document.getElementById('msgContent').scrollHeight);
	}

	// 检查是否已销毁
	var stamp = $("#" + seq + "_msg_body").attr('body-stamp');
	var otherId = $("#" + seq + "_msg_body").attr('uid');
	if (typeof(stamp) == "string" && stamp != "") {
		Message4Js.checkSendSecretDestroy(stamp, otherId);
	}
}

// 隐藏发送的密信
function hideSendBody(seq) {
	$("#" + seq + "_msg_body").css("display", "none");
	
	$("#" + seq + "_secret").html("<a href=\"#\" onclick=\"showSendBody('" + seq +"')\">"+LANG_DICT["CLICK_VIEW"]+"</a>" + 
								  "<img src='qrc:/html/images/lock.png'/>");
}

// 设置发送的密信已读
function setSendSecretRead(stamp) {
	var secretType = $("[body-stamp='"+stamp+"']").attr('secret-type');
	if (secretType != null && typeof(secretType) == "string" ) {
		if (secretType == "text") {
			setSendTextSecretRead(stamp);
		} else if (secretType == "voice") {
			setSendVoiceSecretRead(stamp);
		}
	}
}

// 发送的文字密信已读
function setSendTextSecretRead(stamp) {
	$("[body-stamp='"+stamp+"']").css("display", "none");
	$("[body-stamp='"+stamp+"']").empty();
	$("[secret-stamp='"+stamp+"']").html("<img src='qrc:/html/images/text.png'/>" + "<span>"+LANG_DICT["DESTROYED"]+"</span>");
}

// 发送的语音密信已读
function setSendVoiceSecretRead(stamp) {
	// 获取音频的uuid，如果在播放，则需要停止
	var audioPlaySecretId = $("[body-stamp='"+stamp+"']>div.audioplaysecret").attr('id');
	if (audioPlaySecretId != null && typeof(audioPlaySecretId) == 'string' && audioPlaySecretId.length > 0) {
		var index = audioPlaySecretId.indexOf('_audioplaysecret');
		if (index > 0) {
			var voiceUuid = audioPlaySecretId.substr(0, index);
			if (Message4Js.playingAmrUuid == voiceUuid) {
				Message4Js.jsdebug("stop playing sended secret voice: " + voiceUuid);
				stopplayAmr(voiceUuid);
			}
		}
	}
	$("[body-stamp='"+stamp+"']").css("display", "none");
	$("[body-stamp='"+stamp+"']").empty();
	$("[secret-stamp='"+stamp+"']").css("display", "block");
}

// 获取密信显示的秒数
function getSecretTextReadTime(messageText) {
	var len = messageText.length;
	// 每50个字10秒钟
	var timeUnit = 10;
	var timeSum = Math.ceil(len/50);
	timeSum = timeSum * 10;
	return timeSum;
}

// 接收的密信
function createRecvSecret(obj) {
	if (obj === null) {
		return null;
	}
	
	if (obj.stamp === "") {
		return null;
	}
	
	var voice = isMsgVoice(obj);
	
	var read = isMsgRead(obj);
	
	var ret = null;
	if (!voice) {
		var plainText = obj.plaintext;
		var readTime = getSecretTextReadTime(plainText);
		ret = $("<div/>").addClass("secret").attr("id", obj.stamp + "_secret").attr("read-time", ""+readTime).attr("read", "0");
	} else {
		if (!read) {
			ret = $("<div/>").addClass("secret").attr("id", obj.stamp + "_secret").css("display", "none");
		} else {
			ret = $("<div/>").addClass("secret").attr("id", obj.stamp + "_secret");
		}
	}
	
	if (!voice) {
		if (!read) {
			ret.append("<a href=\"#\" onclick=\"showRecvBody('" + obj.stamp +"')\">"+LANG_DICT["CLICK_VIEW"]+"</a>");
			ret.append("<img src='qrc:/html/images/lock.png'/>");
		} else {
			ret.append("<img src='qrc:/html/images/text.png'/>");
			ret.append("<span>"+LANG_DICT["DESTROYED"]+"</span>");
		}
	} else {
		ret.append("<img src='qrc:/html/images/voice.png'/>");
		ret.append("<span>"+LANG_DICT["DESTROYED"]+"</span>");
	}

	return ret;
}

// 查看接收的密信
function showRecvBody(stamp) {
	var scrollHeight = document.getElementById('msgContent').scrollHeight;
	var scrollTop = document.getElementById('msgContent').scrollTop;
	var clientHeight = document.getElementById('msgContent').clientHeight;

	// 显示消息体，并且改为隐藏操作
	$("#" + stamp + "_msg_body").css("display", "block");

	var read = $("#" + stamp + "_secret").attr("read");
	var leftTime = $("#" + stamp + "_secret").attr("read-time");
	$("#" + stamp + "_secret").html("<a href=\"#\" onclick=\"hideRecvBody('" + stamp +"')\">"+LANG_DICT["CLICK_HIDE"]+"</a>" + 
								  "<img src='qrc:/html/images/second.png'/>" + 
								  "<span class='readtime' id=\""+stamp+"_read_time\">"+leftTime+"\""+"</span>");

	// 如果在最下面则滚动到最下面
	if (scrollTop+clientHeight >= scrollHeight){
		$("#msgContent").scrollTop(document.getElementById('msgContent').scrollHeight);
	}
	
	// 如果还未启动计时，则启动计时
	if (read === "0") {

		var otherId = $("#" + stamp + "_msg_body").attr("uid");

		// 判读是否在其他地方已经读过
		var otherRead = Message4Js.isRecvSecretRead(stamp, otherId);
		if (otherRead) {
			setRecvTextSecretRead(stamp);
			return;
		}

		// 开始计时
		$("#" + stamp + "_secret").attr("read", "1");
		setTimeout("readRecvBodyTimeout('"+stamp+"')", 1000);

		Message4Js.setRecvSecretRead(stamp, otherId);
	}
}

function readRecvBodyTimeout(stamp) {
	var readTime = $("#" + stamp + "_secret").attr("read-time");
	var leftTime = parseInt(readTime);
	if (leftTime > 1) {
		// 时间减一
		leftTime--;
		$("#" + stamp + "_secret").attr("read-time", ""+leftTime);
		$("#" + stamp + "_read_time").html(leftTime+"\"");
		setTimeout("readRecvBodyTimeout('"+stamp+"')", 1000);
	} else {
		// 时间到了销毁
		$("#" + stamp + "_read_time").html(leftTime+"\"");
		setRecvTextSecretRead(stamp);
	}
}

// 隐藏接收的密信
function hideRecvBody(stamp) {
	$("#" + stamp + "_msg_body").css("display", "none");
	
	var leftTime = $("#" + stamp + "_secret").attr("read-time");
	$("#" + stamp + "_secret").html("<a href=\"#\" onclick=\"showRecvBody('" + stamp +"')\">"+LANG_DICT["CLICK_VIEW"]+"</a>" + 
								    "<img src='qrc:/html/images/second.png'/>" + 
								    "<span class='readtime' id=\""+stamp+"_read_time\">"+leftTime+"\""+"</span>");
}

// 接收的文字密信已读
function setRecvTextSecretRead(stamp) {
	$("#" + stamp + "_msg_body").css("display", "none");
	
	$("#" + stamp + "_secret").html("<img src='qrc:/html/images/text.png'/>" + 
								  "<span>"+LANG_DICT["DESTROYED"]+"</span>");
}

// 接收的语音密信已读
function setRecvVoiceSecretRead(stamp) {
	/*
	// 获取音频的uuid，如果在播放，则需要停止
	var audioPlaySecretId = $("#" + stamp + "_msg_body>div.audioplaysecret").attr('id');
	if (audioPlaySecretId != null && typeof(audioPlaySecretId) == 'string' && audioPlaySecretId.length > 0) {
		var index = audioPlaySecretId.indexOf('_audioplaysecret');
		if (index > 0) {
			var voiceUuid = audioPlaySecretId.substr(0, index);
			if (Message4Js.playingAmrUuid == voiceUuid) {
				Message4Js.jsdebug("------------------------------------stop playing recved secret voice: " + voiceUuid);
				AmrPlayer.stop(voiceUuid);
				Message4Js.playingAmrUuid = "";
				amrplayerHandleId = 0;
			}
		}
	}
	*/
	$("#" + stamp + "_msg_body").css("display", "none");
	$("#" + stamp + "_secret").css("display", "block");
}

// 接收的密信设为已读
function onRecvSecretRead(stamp) {
	setRecvTextSecretRead(stamp);
	setRecvVoiceSecretRead(stamp);
}

/******************************** 音频相关 ******************************************************/
// 下载音频完毕
function onAutoDownloadFinish(uuid){

	var scrollHeight = document.getElementById('msgContent').scrollHeight;
	var scrollTop = document.getElementById('msgContent').scrollTop;
	var clientHeight = document.getElementById('msgContent').clientHeight;
	
	var time = $("#"+uuid+"_autodownload").attr("play-time");
	
	var audiostr = "<a class = \"audioplay\" onclick=\"startplayAmr('" + uuid + "')\"></a>";
	audiostr += "<div class = \"audioplayinfo\">" +　time+　"″</div>";
	
	$("#"+uuid+"_autodownload").html(audiostr);

	if(scrollTop+clientHeight >= scrollHeight){
		$("#msgContent").scrollTop(document.getElementById('msgContent').scrollHeight);
	}
}

// 下载音频错误
function onAutoDownloadError(uuid, operator, error){
	Message4Js.jsdebug("JS_________________onAutoDownloadError " + uuid + " " + error);
	$("#"+uuid+"_autodownload").html("<span style=\"color: #FDAC75; font-weight:bold; font-size:12px;\" >"
		+LANG_DICT["VOICE_DOWNLOAD_ERROR"]+"</span><a href=\"#\" onclick=\"restartDownloadAmr('"+uuid+"')\">"+LANG_DICT["RETRY"]+"</a>");
}

// 重新下载语音
function restartDownloadAmr(uuid){
	if (!Message4Js.checkCanTransfer(LANG_DICT["DOWNLOAD_VOICE"]))
		return;
		
	var time = $("#"+uuid+"_autodownload").attr("play-time");
	var filepath = $("#"+uuid+"_autodownload").attr("file-src");
	var str = "<div id=\""+uuid+"_autodownload\" play-time=\""+time+"\" style=\"height:35px;\" file-src=\""+filepath+"\">";
	str += "<span style=\"vertical-align: middle; color:#666;\" >"+LANG_DICT["VOICE_MESSAGE_SPACE"]+
	       "</span><image style=\"vertical-align: middle;\" src=\"qrc:/html/images/loading.gif\" alt=\""+LANG_DICT["DOWNLOADING_VOICE"]+"\" />";
	str +="</div>";
	$("#"+uuid+"_autodownload").html(str);
	startAutoDownload(uuid);
}

// 重新上传语音
function restartUploadAudio(uuid) {

	if (!Message4Js.checkCanTransfer(LANG_DICT["UPLOAD_VOICE"]))
		return;
		
	startAutoUpload(uuid);
}


// 停止播放语音消息
function stopplayAmr(uuid){

	AmrPlayer.stop(uuid);
	var time = $("#"+uuid+"_autodownload").attr("play-time");
	
	var audiostr = "<a class = \"audioplay\" onclick=\"startplayAmr('" + uuid + "')\"></a>";
	audiostr += "<div class = \"audioplayinfo\">" +　time+　"″</div>";
	
	$("#"+uuid+"_autodownload").html(audiostr);
	
	$("#"+uuid+"_audioplaylock").attr("src", "qrc:/html/images/lock.png");

	checkVoiceDestroy(uuid);
	
	Message4Js.playingAmrUuid = "";
	amrplayerHandleId = 0;
}

// 播放语音消息
function startplayAmr(uuid){

	if (checkVoiceOtherRead(uuid))
		return;

	var time = $("#"+uuid+"_autodownload").attr("play-time");
	var filepath = $("#"+uuid+"_autodownload").attr("file-src");

	var audiostr = "<a class = \"audiostop\" onclick=\"stopplayAmr('" + uuid + "')\"></a>";
	    audiostr += "<div class = \"audioplayinginfo\">" +　time+　"″</div>";
	$("#"+uuid+"_autodownload").html(audiostr);
	
	// 停止已经在计时销毁的定时器
	var t = $("#"+uuid+"_audioplaysecret").attr("timeId");
	if (t != null && t != "-1") {
		var timerId = parseInt(t);
		if (t > 0) {
			clearTimeout(timerId);
			$("#"+uuid+"_audioplaysecret").attr("timeId", "-1");
		}
	}

	$("#"+uuid+"_audioplaysecret").html(
		"<img id = \""+uuid+"_audioplaylock\" src='qrc:/html/images/unlock.png'/>"
	);

	amrplayerHandleId = AmrPlayer.play(uuid, filepath);
	Message4Js.playingAmrUuid = uuid;
}

// 播放音频完毕
function onAmrPlayerStopped(uuid, handleId){
	// Message4Js.jsdebug("JS_________________onAmrPlayerStopped " + uuid + " " + handleId + " " + amrplayerHandleId);
	if (amrplayerHandleId != handleId) {
		return;
	}

	var time = $("#"+uuid+"_autodownload").attr("play-time");
	
	var audiostr = "<a class = \"audioplay\" onclick=\"startplayAmr('" + uuid + "')\"></a>";
	audiostr += "<div class = \"audioplayinfo\">" +　time+　"″</div>";
	$("#"+uuid+"_autodownload").html(audiostr);
	
	$("#"+uuid+"_audioplaylock").attr("src", "qrc:/html/images/lock.png");

	checkVoiceDestroy(uuid);
	
	Message4Js.playingAmrUuid = "";
	amrplayerHandleId = 0;
}

function onAmrPlayerError(uuid, err) {
	Message4Js.jsdebug("JS_________________onAmrPlayerError " + uuid + " " + err);
	
	var time = $("#"+uuid+"_autodownload").attr("play-time");
	var audiostr = "<a class = \"audioplay\" onclick=\"startplayAmr('" + uuid + "')\"></a>";
	audiostr += "<div class = \"audioplayinfo\">" +　time+　"″</div>";
	$("#"+uuid+"_autodownload").html(audiostr);
	
	$("#"+uuid+"_audioplaylock").attr("src", "qrc:/html/images/lock.png");

	checkVoiceDestroy(uuid);
	
	Message4Js.playingAmrUuid = "";
	amrplayerHandleId = 0;
}

// 检查语音密信销毁
function checkVoiceDestroy(uuid) {
	var autoDestroy = $("#"+uuid+"_audioplaysecret").attr("auto-destroy");
	if (autoDestroy == "1") { // need destory
		// 只有在未启动定时器的情况下才启动
		var t = $("#"+uuid+"_audioplaysecret").attr("timeId");
		if (t == null || t == "-1") {
			var leftTime = "10";
			$("#"+uuid+"_audioplaysecret").attr("read", "1");
			$("#"+uuid+"_audioplaysecret").attr("read-time", leftTime);
			$("#"+uuid+"_audioplaysecret").html(
				"<img src='qrc:/html/images/second.png'/ style='position: relative; top: 4px;'>" + 
				"<span class='readtime' id=\""+uuid+"_read_time\" style='margin-left: 6px;'>"+leftTime+"\""+"</span>"
				);
			var t = setTimeout("readVoiceTimeout('"+uuid+"')", 1000);
			$("#"+uuid+"_audioplaysecret").attr("timeId", ""+t);
		}
	}
}

// 语音密信销毁计时
function readVoiceTimeout(uuid) {
	var readTime = $("#" + uuid + "_audioplaysecret").attr("read-time");
	var leftTime = parseInt(readTime);
	if (leftTime > 1) {
		// 时间减一
		leftTime--;
		$("#" + uuid + "_audioplaysecret").attr("read-time", ""+leftTime);
		$("#" + uuid + "_read_time").html(leftTime+"\"");
		var t = setTimeout("readVoiceTimeout('"+uuid+"')", 1000);
		$("#"+uuid+"_audioplaysecret").attr("timeId", ""+t);
	} else {
		// 时间到了销毁
		$("#" + uuid + "_read_time").html(leftTime+"\"");
		$("#"+uuid+"_audioplaysecret").attr("timeId", "-1");
		var stamp = $("#" + uuid + "_audioplaysecret").attr("body-stamp");
		setRecvVoiceSecretRead(stamp);
	}
}

// 语音消息是否在别的地方播放过
function checkVoiceOtherRead(uuid) {
	var autoDestroy = $("#"+uuid+"_audioplaysecret").attr("auto-destroy");
	if (autoDestroy == "1") { // need destory
		var isRead = $("#"+uuid+"_audioplaysecret").attr("read");
		if (isRead == "0") {
			var stamp = $("#" + uuid + "_audioplaysecret").attr("body-stamp");
			var otherId = $("#" + stamp + "_msg_body").attr("uid");
			var otherRead = Message4Js.isRecvSecretRead(stamp, otherId);
			if (otherRead) {
				setRecvVoiceSecretRead(stamp);
				return true;
			} else {
				Message4Js.setRecvSecretRead(stamp, otherId);
				return false;
			}
		}
	} else if (autoDestroy == "0") { // secret audio send
		var seq = $("#" + uuid + "_audioplaysecret").attr("audioplay-seq");
		if (typeof(seq) == "string" && seq != "") {
			var stamp = $("#" + seq + "_msg_body").attr('body-stamp');
			var otherId = $("#" + seq + "_msg_body").attr('uid');
			if (typeof(stamp) == "string" && stamp != "") {
				Message4Js.checkSendSecretDestroy(stamp, otherId);
			}
		}
	}

	return false;
}

/***************************** 图片相关 ********************************************/
// 下载图片结束
function onAutoDisplayFinish(uuid,imgsrc,picwid,pichei,diswid,dishei){

	var scrollHeight = document.getElementById('msgContent').scrollHeight;
	var scrollTop = document.getElementById('msgContent').scrollTop;
	var clientHeight = document.getElementById('msgContent').clientHeight;

	var secretFlag = $("[uuid='"+uuid+"_autodisplay']").attr('secret');
	if (secretFlag == null) {
		secretFlag = '0';
	}
	
	var imageDom = "";

	imageDom = "<image uuid='img_"+uuid+"_autodisplay' class='img_autodisplay' title='"+LANG_DICT["DOUBLE_CLICK_VIEW_IMAGE"]+"' ";
	imageDom = imageDom + "secret = '";
	imageDom = imageDom + secretFlag;
	imageDom = imageDom + "' ";
	imageDom = imageDom + "width = '";
	imageDom = imageDom + diswid;
	imageDom = imageDom + "' ";
	imageDom = imageDom + "height = '";
	imageDom = imageDom + dishei;
	imageDom = imageDom + "' ";
	imageDom = imageDom + "origsrc = '";
	imageDom = imageDom + imgsrc;
	imageDom = imageDom + "' ";
	imageDom = imageDom + "exif=\"true\" src='"+Message4Js.getAutodisplayImgSrc(imgsrc)+"' ";
	imageDom = imageDom + "actwidth = '"+picwid+"' ";
	imageDom = imageDom + "actheight = '"+pichei+"' ";
	imageDom = imageDom + "ondblclick=\"openImages('"+imgsrc+"')\"/>";
	
	$("[uuid='"+uuid+"_autodisplay']").html(imageDom);

	if(scrollTop+clientHeight >= scrollHeight){
		$("#msgContent").scrollTop(document.getElementById('msgContent').scrollHeight);
	}
}

// 下载图片错误
function onAutoDisplayError(uuid, operator, error) {
	Message4Js.jsdebug("JS_________________onAutoDisplayError " + uuid + " " + error);
	
	var str ="<image src=\"qrc:/images/errorBmp.png\" alt=\""+LANG_DICT["IMAGE_DOWNLOAD_ERROR"]+"\" onclick=\"restartDownloadPhoto('"+uuid+"')\"/>";
	$("[uuid='"+uuid+"_autodisplay']").html(str);
}

// 重新下载图片
function restartDownloadPhoto(uuid) {

	if (!Message4Js.checkCanTransfer(LANG_DICT["DOWNLOAD_IMAGE"]))
		return;

	var str ="<image src=\"qrc:/images/sendingPic.gif\" alt=\""+LANG_DICT["DOWNLOADING_IMAGE"]+"\" />";
	$("[uuid='"+uuid+"_autodisplay']").html(str);
	
	startAutoDownload(uuid);
}

// 重新上传图片
function restartUploadPhoto(uuid) {

	if (!Message4Js.checkCanTransfer(LANG_DICT["UPLOAD_IMAGE"]))
		return;
	
	startAutoUpload(uuid);
}

/*************************** 附件相关 **********************************************/
// 附件上传终止
function onStopped(uuid, op) {
	
	if (typeof(op) == "string" && op == "upload") {
		stopuploadBack(uuid);
	} else {
		stopdownloadBack(uuid);
	}
}

// 中止上传返回
function stopuploadBack(uuid,ret){
	$("#" + uuid + "_attacheoperatortip").css("display","block");
	$("#" + uuid + "_attacheoperatortip").html(LANG_DICT["SEND_STOPPED"]);
	// 取消发送后不在原来这条重新发送
	// $("#" + uuid + "_tipText").html("<a href=\"javascript:void(0);\" class=\"startupload\" onclick=\"startupload('" + uuid + "')\"></a>");
	$("#" + uuid + "_tipText").html("");
}

// 中止下载返回
function stopdownloadBack(uuid,ret){
	$("#" + uuid + "_attacheoperatortip").css("display","block");
	$("#" + uuid + "_attacheoperatortip").html(LANG_DICT["RECV_STOPPED"]);
	$("#"+uuid+"_tipText").html("<a href=\"javascript:void(0);\" class=\"startdownload\" onclick=\"startdownload('" + uuid + "')\"></a>");
}

// 附件上传下载 结束
function onFinish(uuid, op) {

	$("#"+uuid+"_progresswidth").width("100%");
	
	if (typeof(op) == "string" && op == "download") {
		openFileDialog(uuid);
	}

	$("#"+uuid+"_tipText").html("");
}

function onDirDownloadFinished(uuid) {

	// $("#"+uuid+"_progresswidth").width("100%");
	
	openDirDialog(uuid, true);

	// $("#"+uuid+"_tipText").html("");
}

function onAttachUploadFinish(uuid) {
	
	// $("#"+uuid+"_progresswidth").width("100%");
	
	openFileDirDialog(uuid);

	// $("#"+uuid+"_tipText").html("");
}

function onDirUploadFinished(uuid) {

	// $("#"+uuid+"_progresswidth").width("100%");
	
	openDirDialog(uuid, true);

	// $("#"+uuid+"_tipText").html("");
}

// 文件选项
function openFileDialog(uuid){
	$("#"+uuid+"_process").html("<a href=\"#\" onclick=\"openFile(\'"+uuid+"\')\">"+LANG_DICT["OPEN_FILE"]+"</a>&nbsp;&nbsp;\
		<a href=\"#\" onclick=\"openFileDir(\'"+uuid+"\');\">"+LANG_DICT["OPEN_DIR"]+"</a>&nbsp;&nbsp;\
		<a href=\"#\" onclick=\"fileSaveAs(\'"+uuid+"\');\">"+LANG_DICT["SAVE_AS"]+"</a>");
}

// 文件夹选项
function openDirDialog(uuid, hasCopy) {
	if (hasCopy) {
		$("#"+uuid+"_process").html("<a href=\"#\" onclick=\"openFile(\'"+uuid+"\')\">"+LANG_DICT["OPEN"]+"</a>&nbsp;&nbsp;\
		<a href=\"#\" onclick=\"openFileDir(\'"+uuid+"\');\">"+LANG_DICT["OPEN_DIR"]+"</a>&nbsp;&nbsp;\
		<a href=\"#\" onclick=\"copyDir(\'"+uuid+"\');\">"+LANG_DICT["COPY"]+"</a>");
	} else {
		$("#"+uuid+"_process").html("<a href=\"#\" onclick=\"openFile(\'"+uuid+"\')\">"+LANG_DICT["OPEN"]+"</a>&nbsp;&nbsp;\
		<a href=\"#\" onclick=\"openFileDir(\'"+uuid+"\');\">"+LANG_DICT["OPEN_DIR"]+"</a>");
	}
}

// 打开文件和文件夹选项
function openFileDirDialog(uuid){
	$("#"+uuid+"_process").html("<a href=\"#\" onclick=\"openFile(\'"+uuid+"\')\">"+LANG_DICT["OPEN_FILE"]+
		"</a>&nbsp;&nbsp;<a href=\"#\" onclick=\"openFileDir(\'"+uuid+"\');\">"+LANG_DICT["OPEN_DIR"]+"</a>");
}

// 打开所在文件夹
function openFileDir(uuid){
	Message4Js.openFileDir(uuid);
}

// 打开文件
function openFile(uuid){
	Message4Js.openFile(uuid);
}

// 打开文件夹
function openDir(uuid) {
	Message4Js.openDir(uuid);
}

// 另存为
function fileSaveAs(uuid){
	Message4Js.fileSaveAs(uuid);
}

// 复制文件夹
function copyDir(uuid) {
	Message4Js.copyDir(uuid);
}

// 打开图片
function openImages(imgPath) {
	var allPathes = [];
	$(".img_autodisplay").each(function(){
      allPathes.push($(this).attr("origsrc"));
    });
	Message4Js.openImages(imgPath, allPathes);
}

// 附件传输出错
function onError(uuid, op, err) {
	
	Message4Js.jsdebug("JS_________________onError: " + uuid + " " + op + " " + err);
	
	// 附件
	var str = "";
	if (typeof(op) == "string" && op == "upload") {   
		$("#"+ uuid +"_attacheoperatortip").css("display","block");
		$("#"+ uuid +"_attacheoperatortip").html(LANG_DICT["UPLOAD_ERROR"]);
		// 发送失败后不在原来这条重新发送
		// $("#"+uuid+"_tipText").html("<a href=\"javascript:void(0);\" class=\"startupload\" onclick=\"startupload('" + uuid + "')\"></a>");
		$("#"+uuid+"_tipText").html("");
	} else {
		$("#"+ uuid +"_attacheoperatortip").css("display","block");
		$("#"+ uuid +"_attacheoperatortip").html(LANG_DICT["DOWNLOAD_ERROR"]);
		$("#"+uuid+"_tipText").html("<a href=\"javascript:void(0);\" class=\"startdownload\" onclick=\"startdownload('" + uuid + "')\"></a>");
	}
	
	return true;
}

// 下载
function startdownload(uuid){
	if (!Message4Js.startdownload(uuid))
		return;
		
	$("#"+ uuid +"_attacheoperatortip").css("display","none");
	$("#"+uuid+"_tipText").html("<a href=\"javascript:void(0);\" class=\"stopdownload\" onclick=\"stopdownload('" + uuid + "')\"></a>");		
}

// 中止下载
function stopdownload(uuid){
	Message4Js.stopdownload(uuid);
}

// 中止上传
function stopupload(uuid){
	Message4Js.stopupload(uuid);	
}


/************************ 进度条相关 *********************************************/
// 附件上传下载进度
function onProgress(uuid, percent) {
	
	$("#"+uuid+"_progresswidth").width(percent+"%");

	var tipDisplay = $("#"+ uuid +"_attacheoperatortip").css("display");
	if (tipDisplay == "block") {
		$("#"+ uuid +"_attacheoperatortip").css("display", "none");
	}
	
	return true;
}

/************************ 附件重命名 *********************************************/
// 附件重命名
function onDownloadChanged(uuid, fileName) {
	var fileNameStr = getAttachShowName(fileName);
	$("#"+uuid+"_attname").html(fileNameStr);
	$("#"+uuid+"_attname").attr("title", fileName);
	
	return true;
}

/************************** 清屏 ************************************************/
// 移除所有消息
function RemoveAll() {
	$("div .chatBox_msgList").html("");
	count = 0;
}

// 清屏
function cleanup() {
	// recover more bar and hide
	$('#moreBarContext').unbind();
	$('#moreBar').css('cursor', 'pointer');
	$("#moreBarContext").html("<img id=\"moreTipImg\" src=\"qrc:/html/images/more_msg.png\" style=\"margin-bottom:-3px\">"+LANG_DICT["SPACE_CHECK_MORE_MESSAGE"]);
	$('#moreBarContext').bind("click", showMoreMsg);
	$("#moreBar").css("display", "none");
	
	// remove all messages
	RemoveAll();
}


// 界面已经显示好了。
function initUIComplete() {
	$("#msgContent").scrollTop(document.getElementById('msgContent').scrollHeight);
}


/*************************** 其他 *********************************************/
// 打开URL链接
function openLinkUrl(url){
	Message4Js.openLinkUrl(url);
}

// 链接正则后替换的样式
function replaceText(a,b,c){
	return '<a target="_blank" href="'+ (b == undefined ? "http://" : b) + c+'" >'+a+'</a>';
}

// 链接的正则匹配
function replaceurl (msg){

	var re = /(http:\/\/|https:\/\/|ftp:\/\/|rtsp:\/\/|mms:\/\/|www\.)([\/=\?%\-&_~`@[\]\':+!A-Za-z0-9]*([^\s\u0080-\uffff\'\"\<\>\|])*)/g;
	var abc = msg;
	abc = abc.replace(re, function(a,b,c){
		if (a[a.length-1] == ';') {
			var rightPart = ';';
			var urlLen = a.length-1;
			for (var z = a.length-2; z >= 0; --z) {
				if (a[z] == ';') {
					urlLen = z;
					rightPart += ';';
				} else {
					break;
				}
			}
			return "<a href=\"#"+a.substr(0, urlLen)+"\" onclick=\"openLinkUrl(\'" +a.substr(0, urlLen)+ "\')\">" +a.substr(0, urlLen)+ "</a>" + rightPart;
		} else {
			return "<a href=\"#"+a+"\" onclick=\"openLinkUrl(\'" +a+ "\')\">" +a+ "</a>";
		}
	});

	return abc;
}

// 扩展字符串的替换方法
String.prototype.replaceAll = function(s1,s2) { 
	/*
	return this.replace(new RegExp(s1,"gm"),s2);
	*/
	var index = 0;
	var searchFrom = 0;
	var searchStr = this;
	var replacedStr = '';

	index = searchStr.indexOf(s1, searchFrom);
	while (index != -1) {
		replacedStr += searchStr.substring(searchFrom, index);
		replacedStr += s2;

		searchFrom = index+s1.length;
		index = searchStr.indexOf(s1, searchFrom);
	}

	replacedStr += searchStr.substring(searchFrom);

	return replacedStr;
}

// 显示更多消息提示栏
function showMoreMsgTip(){
	$("#moreBar").css("display","block");
}

// 隐藏更多消息提示栏
function closeMoreMsgTip(){
	$("#moreBar").css("display","none");
}

// 显示查看历史消息的提示栏
function showMoreHistoryMsgTip(){
	// 去除点击事件
	$('#moreBarContext').unbind();

	$('#moreBar').css('cursor', 'default');

	$("#moreBarContext").html("<span style=\"color: #333333; \">"+LANG_DICT["MORE_MESSAGE_IN_HISTORY_COMMA"]+
		                      "</span><a href=\"#\" onclick=\"openHistory()\">"+LANG_DICT["OPEN_CHAT_HISTORY"]+"</a>");
}

// 打开消息记录
function openHistory() {
	$("#moreBarContext").html("<span style=\"color: #333333; \">"+LANG_DICT["MORE_MESSAGE_IN_HISTORY"]+"</span>");
	Message4Js.openHistory();
}


// 加载更多消息
function showMoreMsg(){
	// 去除点击事件
	$('#moreBarContext').unbind();

	// 图标的切换
	$("#moreTipImg").attr("src","qrc:/html/images/loading_image_small.gif");

	// 记录当前层的位置
	$fdiv = $(".chatBox_msgList div:first");

	// 加载消息
	Message4Js.getHistoryMsg();
}

function showMoreMsgFinish(){
	var moreBarDisplay = $("#moreBar").css("display");
	if (moreBarDisplay != "block") {
		return;
	}

	if($fdiv[0]){
		// 滚动位置控制
		$("#msgContent").scrollTop($fdiv.offset().top);
	}

	// 图标切换
	$("#moreTipImg").attr("src","qrc:/html/images/more_msg.png");

	// 添加点击事件
	$('#moreBarContext').bind("click",showMoreMsg);
	
	// 更新消息位置
	msgContentScrollTop = -3;
}

// 点击@文本
function onClickAtText(atText){
	Message4Js.onClickAtText(atText);
}

// 点击提示消息的动作
function tipActionClicked(param, aLink){
	if (Message4Js.onClickTipAction(param))
	{
		$(aLink).remove(); // remove this action
	}
}
