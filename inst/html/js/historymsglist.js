
// 公共变量
var maxMsgCount = 20;
var count = 0;
var picArray = new Array("doc","docx","xls","xlsx","ppt","pptx","gif","jpeg","jpg","png","bmp","pdf","swf","zip","rar","txt","avi","wma","rmvb","rm","mp4","3gp");
var bodyHeight; 
var amrplayerHandleId = 0;

var kAttachAutoDownloadType = "auto-download";
var kAttachAutoDisplayType  = "auto-display";
var kAttachDirType          = "dir";

var kAttachSuccessful       = "successful";
var kAttachCancel           = "cancel";
var kAttachError            = "error";      

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

$(document).ready(function(){	

	// 消息高亮显示
	$(".msgContainer").die().live("click",function(){
		clearMsgFocus();
		$(this).attr("class","msgContainer_focus");
	});

});

$(document).ready(function () {
	
	try {

		setCurLanguage();

		count = $(".chatBox_msgList > div").size();
		maxMsgCount = Message4Js.maxMsgCount;

		// 消息展示
		Message4Js.displaymsg.connect(displaymsg);
		Message4Js.displaymsgs.connect(displaymsgs);
		Message4Js.displayTipMsg.connect(displayTipMsg);
		Message4Js.focusMsg.connect(focusMsg);
		Message4Js.highlightKeyword.connect(highlightKeyword);
		Message4Js.displayNoMessage.connect(displayNoMessage);
				
		// 音频下载		
		Message4Js.onAutoDownloadFinish.connect(onAutoDownloadFinish);
		Message4Js.onAutoDownloadError.connect(onAutoDownloadError);
		AmrPlayer.stopped.connect(onAmrPlayerStopped);
		AmrPlayer.error.connect(onAmrPlayerError);

		// 图片下载
		Message4Js.onAutoDisplayFinish.connect(onAutoDisplayFinish);
		Message4Js.onAutoDisplayError.connect(onAutoDisplayError);

		// 附件下载
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

		// 密信相关
		Message4Js.setSendSecretRead.connect(setSendSecretRead);
		Message4Js.onRecvSecretRead.connect(onRecvSecretRead);

		// 滚动
		Message4Js.scrollToBottom.connect(scrollToBottom);
		Message4Js.scrollToTop.connect(scrollToTop);

	} catch(e) {
		Message4Js.jsdebug("history load exeption"  + e);
	}		
});

// 清除消息高亮显示
function clearMsgFocus(){
	$(".msgContainer_focus").each(function(){
		$(this).attr("class","msgContainer");
	});
}

// 高亮某条消息
function focusMsg(msgId){
	clearMsgFocus();
	$("div#"+msgId).attr("class", "msgContainer_focus");
}

// 高亮关键字
function highlightKeyword(keyword){
	$(".msgBody>div").highlight(keyword);
}

// 调整页面大小
function onresize(){
	bodyHeight = document.documentElement.clientHeight;
	$("#msgContent").height(bodyHeight);
}

// 页面加载
function onload(){

	onresize();
	
	Message4Js.loadFinished();
}


// 移除最顶层的一个消息
function RemoveFirst() {
	$(".chatBox_msgList div:first").remove();
	count = count - 1;
}

// 查看某条消息的前后消息
function openMsgContext(messageId, msgType, otherId) {
	Message4Js.openMsgContext(messageId, msgType, otherId);
}

// 创建消息标题
function createTitle(obj) {

	if (obj === null) {
		return null;
	}

	var title, content, time;
	if ( typeof obj.time !== "string" || obj.time == null) {
		var d = new Date();
		time = d.getFullYear() + "-" + (d.getMonth() + 1) + "-" + d.getDate() + " " + d.toLocaleTimeString();
	} else {
		time = obj.time;
	}

	if (obj.method === null || ( typeof obj.method === "string" && obj.method === "Method_Send")) {
		if (obj.gid !== null && typeof obj.gid === "string" && obj.gid !== "") {
			content = (obj.uname === null||obj.uname == "" ? obj.uid : obj.uname);
		} else {
			content = (Message4Js.uname === null || Message4Js.uname == "" ? Message4Js.uid : Message4Js.uname);
		}
		title = content + "(" + (Message4Js.uid === null ? "" : Message4Js.uid) + ")";
	} else {
		content = (obj.uname === null||obj.uname == "" ? obj.uid : obj.uname);
		title = content + (obj.uid === null ? "" : "(" + obj.uid + ")");
	}

	var ret = $("<div/>").addClass("msgHead");
	var span = $("<span/>");
	span.attr("title", title);
	span.text(content);
	ret.append(span);

	span = $("<span/>");
	span.css("margin-left", "8px");
	span.text(time);
	ret.append(span);

	var otherId = obj.uid;
	if (obj.gid !== null && typeof obj.gid === "string" && obj.gid !== "") {
		otherId = obj.gid;
	}
	ret.append("<a class='context_anchor' name='"+obj.messageid+"' href='#' style='float: right; display: none;' onclick=\"openMsgContext(\'" 
		       +obj.messageid+ "\', \'"+obj.msgtype+"\', \'"+otherId+"\')\">" +LANG_DICT["LOOK_MESSAGE_CONTEXT"]+ "</a>")

	return ret;
}

// 获取图片大小
function getAutodisplaySizeByUrl(url, widthHint, heightHint) {
	var obj = Message4Js.getAutodisplaySizeByUrl(url, widthHint, heightHint);
	return obj;
}

// 对自动播放图片的处理
function dealAutoDisplay(context,obj){
	
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

		if(value.type != undefined && value.type == kAttachAutoDisplayType) {

			// 自动显示处理(图片)
			if (direct == "send") {
				var path = ((value.path != undefined) ? value.path : "");
				var size = getAutodisplaySizeByUrl(path, value.picwidth, value.picheight);
				var width = size.width;
				var height = size.height;
				if (width > 0 && height > 0) {
					var str = "<div uuid='"+value.uuid+"_autodisplay' style=\"padding-top:3px;\" secret=\""+secretFlag+"\">";
					str +="<image class='img_autodisplay' title='"+LANG_DICT["DOUBLE_CLICK_VIEW_IMAGE"]+"' origsrc=\""+value.path+"\" src=\""
					    +Message4Js.getAutodisplayImgSrc(value.path)+"\" width=\""+width+"\" height=\""+height+"\" secret=\""
					    +secretFlag+"\" ondblclick=\"openImages('"+value.path+"')\" />";
					str +="</div>";
					context = context.replaceAll("{"+value.uuid+"}",str);
				} else {
					var str = "<div uuid='"+value.uuid+"_autodisplay' style=\"padding-top:3px;\" secret=\""+secretFlag+"\">";
					str +="<image src=\"qrc:/images/errorBmp.png\" alt=\""+LANG_DICT["IMAGE_NOT_EXIST"]+"\""
					str +=" />";
					str +="</div>";
					context = context.replaceAll("{"+value.uuid+"}",str);
				}
			} else {
				var imageOK = false;
				if (value.result != undefined && value.result == kAttachSuccessful) {
					var path = ((value.path != undefined) ? value.path : "");
				 	var size = getAutodisplaySizeByUrl(path, value.picwidth, value.picheight);
					var width = size.width;
					var height = size.height;
					if (width > 0 && height > 0) {
						var str = "<div uuid='"+value.uuid+"_autodisplay' style=\"padding-top:3px;\" secret=\""+secretFlag+"\">";
						str +="<image class='img_autodisplay' title='"+LANG_DICT["DOUBLE_CLICK_VIEW_IMAGE"]+"' origsrc=\""+value.path
						    +"\" src=\""+Message4Js.getAutodisplayImgSrc(value.path)+"\" width=\""+width+"\" height=\""+height
						    +"\" secret=\""+secretFlag+"\" ondblclick=\"openImages('"+value.path+"')\" />";
						str +="</div>";
						context = context.replaceAll("{"+value.uuid+"}",str);

						imageOK = true;
					}
				} 

				// 如果图片没有下载完成，则点击重新下载
				if (!imageOK) {
					var str = "<div uuid='"+value.uuid+"_autodisplay' style=\"padding-top:3px;\" secret=\""+secretFlag+"\">";
					str +="<image src=\"qrc:/images/errorBmp.png\" alt=\""+LANG_DICT["IMAGE_NOT_EXIST"]+"\""
					str += "onclick=\"restartDownloadPhoto('"+value.uuid+"')\"";
					str +=" />";
					str +="</div>";
					context = context.replaceAll("{"+value.uuid+"}",str);
				}
			}
		}
	});
	
	return context;
}

// 打开URL链接
function openLinkUrl(url){
	Message4Js.openLinkUrl(url);
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

// 创建消息体
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
		
		// 表情的处理
		var context = Message4Js.parseFaceSymbol(context);

		// 对自动播放图片的处理
		context = dealAutoDisplay(context, obj);

		// 处理超链接
		context = replaceurl(context);
		
		if (typeof obj.share_url == "string" && obj.share_url != "") {
			context += "<br/>"+LANG_DICT["SHARE_FROM_SPACE"];
			context += "<a href=\"#\" onclick=\"openLinkUrl(\'" +obj.share_url+ "\')\">" +obj.share_url+ "</a>";
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

// 另存为
function fileSaveAs(uuid){
	Message4Js.fileSaveAs(uuid);
}

// 打开文件
function openFile(uuid){
	Message4Js.openFile(uuid);
}

// 打开所在文件夹
function openFileDir(uuid){
	Message4Js.openFileDir(uuid);
}

// 打开文件夹
function openDir(uuid) {
	Message4Js.openDir(uuid);
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

// 获取附件信息
function getAttachments(obj,direct){
	
	var attachments ="";

	$.each(obj.attachs, function(n,value) {
	
		if(value.type != undefined && value.type == kAttachAutoDisplayType){	

		}else if(value.type != undefined && value.type == kAttachAutoDownloadType){
			
			if (direct == 'receive') { // audio received

				var str = "";
				if (value.result != kAttachSuccessful) {
					str = "<div id=\""+value.uuid+"_autodownload\" play-time=\""+value.time+"\" style=\"padding-top:3px;height:35px;\" file-src=\""+value.path+"\">";
					str += "<span style=\"color: #FDAC75; font-weight:bold; font-size:12px;\" >"+LANG_DICT["VOICE_DOWNLOAD_ERROR"]+
					       "</span><a href=\"#\" onclick=\"restartDownloadAmr('"+value.uuid+"')\">"+LANG_DICT["RETRY"]+"</a>";
					str += "</div>";
				} else {
					str = "<div id=\""+value.uuid+"_autodownload\" play-time=\""+value.time+"\" style=\"padding-top:3px;height:35px;\" file-src=\""+value.path+"\">";
					var audiostr = "<a class = \"audioplay\" onclick=\"startplayAmr('" + value.uuid + "')\"></a>";
					audiostr += "<div class = \"audioplayinfo\">" + value.time +　"″</div>";
					str += audiostr;
					str +="</div>";
				}

				if (isMsgSecret(obj)) {
					var autoDestroy = "1";
					if (obj.sync == 1) {
						autoDestroy = "0";
					}
					str += "<div class = \"audioplaysecret\" id = \""+value.uuid+"_audioplaysecret\" body-stamp = \""+obj.stamp+"\" auto-destroy = \""+autoDestroy+"\" read = \"0\" read-time = \"10\">" + 
						   "<img id = \""+value.uuid+"_audioplaylock\" src='qrc:/html/images/lock.png'/>" + 
						   "</div>";
				}
					
				attachments = attachments + str;
				
			} else { // audio send
			
				var str = "<div id=\""+value.uuid+"_autodownload\" play-time=\""+value.time+"\" style=\"padding-top:3px;height:35px;\" file-src=\""+value.path+"\">";
				var audiostr = "<a class = \"audioplay\" onclick=\"startplayAmr('" + value.uuid + "')\"></a>";
				audiostr += "<div class = \"audioplayinfo\">" + value.time + "″</div>";
				str += audiostr;
				str +="</div>";
				if (isMsgSecret(obj)) {
					str += "<div class = \"audioplaysecret\" id = \""+value.uuid+"_audioplaysecret\" auto-destroy = \"0\" audioplay-seq = \""+obj.sequence+"\">" + 
						   "<img id = \""+value.uuid+"_audioplaylock\" src='qrc:/html/images/lock.png'/>" + 
						   "</div>";
				}
				str += "<span id=\""+value.uuid+"_autodownload_tipText\"></span>";
				
				attachments = attachments + str;
			}

		}else{
	
			// 文件大小显示处理
			var fileLength = value.size;
			var fileLengthStr = getAttachSizeString(fileLength);
								
			// 文件名称长度处理
			var fileNameStr = getAttachShowName(value.savedname);
			
			// 文件后缀
			var format = value.format;
			var formatStr = format;
			if(!isConFormat(format)){
				formatStr = "other";
			}
			
			var tipText = "";
			if(direct == 'send'){
				tipText = "";
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
					str += "		<img src='qrc:/html/images/file_formats/folder.png'/>";
				} else {
					str += "		<img src='qrc:/html/images/file_formats/"+formatStr+".png'/>";
				}
				str += "		</div>";
				str += "		<div style='float:left; margin-left: 10px;'>";
				str += "			<span style='color:#666666;' id=\""+value.uuid+"_attname\" title=\""+value.savedname+"\">"+fileNameStr+"</span>";
				str += "			<br/>";
				str += " 			<span style='color:#a0a0a0;'>"+fileLengthStr+"&nbsp;&nbsp;</span>";
				str += "		</div>";
				str += "	</div>";
				str += "	<div id=\""+value.uuid+"_process\">";
			if(direct == 'receive' && value.result != undefined && value.result != kAttachSuccessful){
				str += "		<div class=\"progress\">";
				str += "			<span id = \""+ value.uuid +"_progresswidth\" class=\"blue\" style=\"width: 0%;\"><span></span></span>";
				str += "		</div>";
				str += "		<div id = \""+ value.uuid +"_attacheoperatortip\" class=\"attacheoperatortip\">";
				str += "		  &bnsp;";
				str += "		</div>";
				str += "		<div id=\"" + value.uuid + "_tipText\" class=\"attacheoperator\">";
				str += 				tipText;
				str += "		</div>"
			}else{
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
	
	var ret = "";
	
	var attach = "";
	if ((obj.method === null || ( typeof obj.method === "string" && obj.method === "Method_Send")) && obj.sync != 1) { // 发送
		attach = getAttachments(obj,"send");
	} else { // 接收
		attach = getAttachments(obj,"receive");
	}

	if (attach != "") {

		if (obj.method === null || ( typeof obj.method === "string" && obj.method === "Method_Send")) { // 发送

			if (typeof obj.sequence == "string" && obj.sequence != "") {

				ret = $("<div/>").addClass("msgBody").attr("id", obj.sequence + "_msg_body").attr("uid", obj.uid)
					.attr('body-stamp', obj.stamp).attr('secret-type', 'voice');
				
				var read = isMsgRead(obj);
				if (!read) {
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

// 显示空消息
function displayNoMessage() {
	RemoveAll();
	var emptyContainer = $("<div/>").addClass('empty').css('text-align', 'center').css('padding-top', '36px');
	emptyContainer.append("<img src='qrc:/html/images/notmsg.png' />");
	emptyContainer.append("<div style='color: #8f8f8f;'>"+LANG_DICT["NO_CHAT_HISTORY"]+"</div>")
	$(".chatBox_msgList").append(emptyContainer);
}

// 生成消息来源
function makeMsgSource(obj) {
	if (obj === null) {
		return null;
	}

	var needCreateSource = true;
	var msgType = obj.msgtype;
	var otherId = obj.uid;
	if (obj.gid !== null && typeof obj.gid === "string" && obj.gid !== "") {
		otherId = obj.gid;
	}

	var msgSources = $(".msgsource");
	if (msgSources.size() > 0) {
		var lastMsgSource = msgSources.last();
		var lastMsgType = lastMsgSource.attr("msgtype");
		var lastMsgOtherId = lastMsgSource.attr("otherid");
		
		// 如果上次显示的消息来源是相同的，则不用再添加
		if ((lastMsgType == msgType) && (lastMsgOtherId == otherId)) {
			needCreateSource = false;
		}
	}

	if (needCreateSource) {
		var msgSource = $("<div/>").addClass("msgsource").attr("msgtype", msgType).attr("otherId", otherId);
		var msgSourceStr = Message4Js.messageSource(msgType, otherId);
		msgSource.append("<span style='color: #808080;'>"+msgSourceStr+"</span>");
		return msgSource;
	}

	return null;
}


// 对话消息的展示
function displaymsg(obj) {

	try {

		if (obj === null) {
			return false;
		}

		// 消息数量
		if (count >= maxMsgCount) {
			RemoveFirst();
		}

		// 显示消息来源
		if (Message4Js.enableMsgSource) {
			var msgSource = makeMsgSource(obj);
			if (msgSource !== null) {
				$(".chatBox_msgList").append(msgSource);	
			}
		}
		
		var msgContainer = $("<div/>");
		msgContainer.addClass("msgContainer");
		msgContainer.attr('id', obj.messageid);
		msgContainer.attr('msgtype', obj.msgtype);
		if (isMsgSecret(obj)) {
			msgContainer.attr('secret', '1');
		} else {
			msgContainer.attr('secret', '0');
		}

		// 消息
		var msg = $("<div/>");
		var msgType;
		if (obj.method === null || ( typeof obj.method === "string" && obj.method === "Method_Send")) {
			msgType = "owner"; 
			msg.addClass("chatBox_myMsg");
		} else {
			msgType = "other"; 
			if (obj.uid == Message4Js.uid && obj.uid !== null) {
				msg.addClass("chatBox_myMsg");
			} else {
				msg.addClass("chatBox_buddyMsg");
			}
		}

		// 标题
		var header = createTitle(obj);
		if (header !== null) {
			msg.append(header);
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
			msg.append(secret);
		}
		
		// 消息体
		var content = createMsgBody(obj, (msgType == "owner"));
		if (content !== null) {
			msg.append(content);
		}

		// 附件
		var attachment = createAttachment(obj);
		if (attachment !== null) {
			msg.append(attachment);
		}
		
		// 附件
		msgContainer.append(msg);

		// 悬浮显示
		msgContainer.hover(function() {
			/* Stuff to do when the mouse enters the element */
			if (Message4Js.enableOpenContextMsg) {
				var msgId = $(this).attr('id');
				$('div#'+msgId+' a.context_anchor').show();
			}
		}, function() {
			/* Stuff to do when the mouse leaves the element */
			if (Message4Js.enableOpenContextMsg) {
				var msgId = $(this).attr('id');
				$('div#'+msgId+' a.context_anchor').hide();
			}
		});
		
		$(".chatBox_msgList").append(msgContainer);
		count++;
		
	} catch (e) {
		Message4Js.jsdebug("JS_________________displaymsg " + e);
	}

	return true;
}


// 	消息组的展示	
function displaymsgs(obj) {

	$.each(obj, function(n,value) {
		displaymsg(value);
	});
}


// 消息展示
function displayTipMsg(isSend, displaytime, infomsg, level, action, param, obj) {

	if (displaytime == null || infomsg == null || level == null){
		return false;
	}

	// 消息数量
	if (count >= maxMsgCount) {
		RemoveFirst();
	}

	var msgContainer = $("<div/>");
	msgContainer.addClass("msgContainer");
	msgContainer.attr('id', obj.messageid);
	msgContainer.attr('msgtype', obj.msgtype);

	var msg = $("<div/>");
	msg.addClass("chatBox_infoMsg");

	var tipHead = $("<div/>").addClass("msgHead");
	var span = $("<span/>");
	span.text(displaytime);
	tipHead.append(span);
	
	// 提示图标
	var ret = "";
	var tipImage = "";
	if (level == "info") {
	    tipImage = "<img style='vertical-align: text-bottom;' src='qrc:/html/images/tip_info.png' />";
	}
	else if (level == "warn") {
		tipImage = "<img style='vertical-align: text-bottom;' src='qrc:/html/images/tip_warn.png' />";	
	}
	else if (level == "error") {
		tipImage = "<img style='vertical-align: text-bottom;' src='qrc:/html/images/tip_error.png' />";	
	}
	else {
		tipImage = "<img style='vertical-align: text-bottom;' src='qrc:/html/images/tip_ok.png' />";
	}
	
	var content = tipImage;
	content += "<span style='margin-left: 2px;'>" + infomsg + "</span>";
	
	// 链接处理
	if (action != null && action != "") {
		var tipAnchor = "<a href=\"#\" onclick=\"tipActionClicked(\'" +param+ "\')\">" +action+ "</a>";
		content += tipAnchor;
	}

	var tipBody = $("<div/>").addClass("msgBody");
	var bodyDiv = $("<div/>");
	tipBody.append(bodyDiv);
	bodyDiv.html(content);
	
	// 组装提示消息
	msg.append(tipHead);
	msg.append(tipBody);

	msgContainer.append(msg);
	
	$(".chatBox_msgList").append(msgContainer);
		count++;

	return true;
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
	$("#" + seq + "_msg_body").css("display", "block");
	
	$("#" + seq + "_secret").html("<a href=\"#\" onclick=\"hideSendBody('" + seq +"')\">"+LANG_DICT["CLICK_HIDE"]+"</a>" + 
								  "<img src='qrc:/html/images/unlock.png'/>");

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
	$("#" + stamp + "_msg_body").css("display", "block");
	
	var read = $("#" + stamp + "_secret").attr("read");
	var leftTime = $("#" + stamp + "_secret").attr("read-time");
	$("#" + stamp + "_secret").html("<a href=\"#\" onclick=\"hideRecvBody('" + stamp +"')\">"+LANG_DICT["CLICK_HIDE"]+"</a>" + 
								  "<img src='qrc:/html/images/second.png'/>" + 
								  "<span class='readtime' id=\""+stamp+"_read_time\">"+leftTime+"\""+"</span>");
	
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
	$("#" + stamp + "_msg_body").css("display", "none");
	$("#" + stamp + "_secret").css("display", "block");
}

// 接收的密信设为已读
function onRecvSecretRead(stamp) {
	setRecvTextSecretRead(stamp);
	setRecvVoiceSecretRead(stamp);
}

/******************************** 音频相关 ******************************************************/
// 音频下载结束
function onAutoDownloadFinish(uuid){

	var time = $("#"+uuid+"_autodownload").attr("play-time");
	
	var audiostr = "<a class = \"audioplay\" onclick=\"startplayAmr('" + uuid + "')\"></a>";
	audiostr += "<div class = \"audioplayinfo\">" +　time+　"″</div>";
	
	$("#"+uuid+"_autodownload").html(audiostr);
}

// 音频下载错误
function onAutoDownloadError(uuid, operator, error){
	Message4Js.jsdebug("JS_________________onAutoDownloadError " + uuid + " " + error);
	$("#"+uuid+"_autodownload").html("<span style=\"color: #FDAC75; font-weight:bold; font-size:12px;\" >"+LANG_DICT["VOICE_DOWNLOAD_ERROR"]+
		                             "</span><a href=\"#\" onclick=\"restartDownloadAmr('"+uuid+"')\">"+LANG_DICT["RETRY"]+"</a>");
}

// 重下载语音
function restartDownloadAmr(uuid){
	if (!Message4Js.checkCanTransfer(LANG_DICT["DOWNLOAD_VOICE"]))
		return;
		
	var time = $("#"+uuid+"_autodownload").attr("play-time");
	var filepath = $("#"+uuid+"_autodownload").attr("file-src");
	var str = "<div id=\""+uuid+"_autodownload\" play-time=\""+time+"\" style=\"padding-top:3px;\" file-src=\""+filepath+"\">";
	str +="<span style=\"vertical-align: middle; color:#666;\" >"+LANG_DICT["VOICE_MESSAGE_SPACE"]
	     +"</span><image style=\"vertical-align: middle;\" src=\"qrc:/html/images/loading.gif\" alt=\""
	     +LANG_DICT["DOWNLOADING_VOICE"]+"\" />";
	str +="</div>";
	$("#"+uuid+"_autodownload").html(str);
	startAutoDownload(uuid);
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
// 图片下载结束
function onAutoDisplayFinish(uuid,imgsrc,picwid,pichei,diswid,dishei){

	var scrollHeight = document.getElementById('msgContent').scrollHeight;
	var scrollTop = document.getElementById('msgContent').scrollTop;
	var clientHeight = document.getElementById('msgContent').clientHeight;

	var secretFlag = $("[uuid='"+uuid+"_autodisplay']").attr('secret');
	if (secretFlag == null) {
		secretFlag = '0';
	}
	
	var imageDom = "";

	imageDom = "<image uuid='img_"+uuid+"_autodisplay'  class='img_autodisplay' title='"+LANG_DICT["DOUBLE_CLICK_VIEW_IMAGE"]+"' ";
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

// 图片下载错误
function onAutoDisplayError(uuid, operator, error) {
	Message4Js.jsdebug("JS_________________onAutoDisplayError " + uuid + " " + error);
	
	var str ="<image src=\"qrc:/images/errorBmp.png\" alt=\""+LANG_DICT["IMAGE_DOWNLOAD_ERROR"]+"\" onclick=\"restartDownloadPhoto('"+uuid+"')\"/>";
	$("[uuid='"+uuid+"_autodisplay']").html(str);
}

// 重新下载图片
function restartDownloadPhoto(uuid){

	if (!Message4Js.checkCanTransfer(LANG_DICT["DOWNLOAD_IMAGE"]))
		return;

	var str ="<image src=\"qrc:/images/sendingPic.gif\" alt=\""+LANG_DICT["DOWNLOADING_IMAGE"]+"\" />";
	$("[uuid='"+uuid+"_autodisplay']").html(str);

	startAutoDownload(uuid);
}

// 附件自动下载
function startAutoDownload(uuid){
	Message4Js.startAutoDownload(uuid);
}

/*************************** 附件相关 **********************************************/
// 附件下载完后的打开、另存为提示
function openFileDialog(uuid){
	var path = $("#"+uuid+"_process").attr("fpath");
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

// 附件上传下载结束
function onFinish(uuid, op) {
	
	$("#"+uuid+"_progresswidth").width("100%");
	
	if (typeof(op) == "string" && op == "download") {
		openFileDialog(uuid);
	}

	$("#"+uuid+"_tipText").html("");
}

function onDirDownloadFinished(uuid) {

	$("#"+uuid+"_progresswidth").width("100%");
	
	openDirDialog(uuid, true);

	$("#"+uuid+"_tipText").html("");
}

function onAttachUploadFinish(uuid) {

	$("#"+uuid+"_progresswidth").width("100%");
	
	openFileDirDialog(uuid);

	$("#"+uuid+"_tipText").html("");
}

function onDirUploadFinished(uuid) {

	$("#"+uuid+"_progresswidth").width("100%");
	
	openDirDialog(uuid, true);

	$("#"+uuid+"_tipText").html("");
}

// 打开文件和文件夹选项
function openFileDirDialog(uuid){
	$("#"+uuid+"_process").html("<a href=\"#\" onclick=\"openFile(\'"+uuid+"\')\">"+LANG_DICT["OPEN_FILE"]
		                        +"</a>&nbsp;&nbsp;<a href=\"#\" onclick=\"openFileDir(\'"+uuid+"\');\">"+LANG_DICT["OPEN_DIR"]+"</a>");
}

// 附件传输出错
function onError(uuid, op, err) {

	Message4Js.jsdebug("JS_________________onError: " + uuid + " " + op + " " + err);
	
	// 附件
	var str = "";
	if (typeof(op) == "string" && op == "upload") {   
		/*
		history do not has upload
		*/
	} else {
		$("#"+ uuid +"_attacheoperatortip").css("display","block");
		$("#"+ uuid +"_attacheoperatortip").html(LANG_DICT["DOWNLOAD_ERROR"]);
		$("#"+uuid+"_tipText").html("<a href=\"javascript:void(0);\" class=\"startdownload\" onclick=\"startdownload('" + uuid + "')\"></a>");
	}
	
	return true;
}


// 中止下载返回
function stopdownloadBack(uuid,ret){
	$("#" + uuid + "_attacheoperatortip").css("display","block");
	$("#" + uuid + "_attacheoperatortip").html(LANG_DICT["RECV_STOPPED"]);
	$("#"+uuid+"_tipText").html("<a href=\"javascript:void(0);\" class=\"startdownload\" onclick=\"startdownload('" + uuid + "')\"></a>");
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

// 附件传输的终止
function onStopped(uuid, op) {
	if (typeof(op) == "string" && op == "upload") {
		/*
		history do not has upload
		*/
	} else {
		stopdownloadBack(uuid);
	}
}


// 附件上传下载进度
function onProgress(uuid, percent) {			

	$("#"+uuid+"_progresswidth").width(percent+"%");

	var tipDisplay = $("#"+ uuid +"_attacheoperatortip").css("display");
	if (tipDisplay == "block") {
		$("#"+ uuid +"_attacheoperatortip").css("display", "none");
	}

	return true;
}

// 附件重命名
function onDownloadChanged(uuid, fileName) {
	var fileNameStr = getAttachShowName(fileName);
	$("#"+uuid+"_attname").html(fileNameStr);
	$("#"+uuid+"_attname").attr("title", fileName);
	
	return true;
}

// 移除所有消息
function RemoveAll() {
	$("div .chatBox_msgList").html("");
	count = 0;
}

// 清屏
function cleanup() {
	RemoveAll();
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

// 点击提示消息的动作
function tipActionClicked(param){
	Message4Js.onClickTipAction(param);
}

/*************************** 页面滚动 **********************************************/
function scrollToBottom() {
	$("#msgContent").scrollTop(document.getElementById('msgContent').scrollHeight);
}

function scrollToTop() {
	$("#msgContent").scrollTop(0);
}