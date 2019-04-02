
// 公共变量的定义 
var bodyHeight; 
var maxMsgCount = 10000;
var count = 0;
var $fdiv;
var picArray = new Array("doc","docx","xls","xlsx","ppt","pptx","gif","jpeg","jpg","png","bmp","pdf","swf","zip","rar","txt","avi","wma","rmvb","rm","mp4","3gp");
var msgContentScrollTop = -3;

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

		// 消息显示
		Message4Js.displaymsg.connect(displaymsg);
		Message4Js.displaymsgAtTop.connect(displaymsgAtTop);

		// 清屏
		Message4Js.cleanup.connect(cleanup);

		// 获取历史消息相关
		Message4Js.showMoreMsgTip.connect(showMoreMsgTip);
		Message4Js.closeMoreMsgTip.connect(closeMoreMsgTip);
		Message4Js.showMoreMsgFinish.connect(showMoreMsgFinish);
		
		// ui ok
		Message4Js.initUIComplete.connect(initUIComplete);
		Message4Js.pageReady.connect(pageReady);

	} catch(e) {
		Message4Js.jsdebug("subscription load exception" + e);
	}

	$('#moreBarContext').bind("click",showMoreMsg);		
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

    // Message4Js.jsdebug("---------------------------------" + direction + "delta: " + event.wheelDelta + " msgContentScrollTop: " + msgContentScrollTop);
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

// 上传文件的类型图片库中是否存在
function isConFormat(format){
	for(var z=0;z<picArray.length;z++){
		if(picArray[z] == format){
			return true;
		}
	}
	return false;
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

// 打开URL链接
function openLinkUrl(url){
	Message4Js.openLinkUrl(url);
}

/**
 * 打开链接
 *
	 * idStr:消息的id
 */
 function openTitle(idStr, messageIdStr) {
	Message4Js.openTitle(idStr, messageIdStr);
 }
 
 /**
 * 打开附件url
 *
	 * url:附件url
 */
 function openAttach(url, name) {
	Message4Js.openAttach(url, name);
 }

  /**
 * 图片下载完成回调
 *
 */
 function onImgLoadComplete(imageObj) {

 	if($fdiv && $fdiv.length > 0){
		// 滚动位置控制
		$("#msgContent").scrollTop($fdiv.offset().top);
	} else {
		// 滚到最下面
		$("#msgContent").scrollTop(document.getElementById('msgContent').scrollHeight);
	}
	
	/*
	Message4Js.jsdebug("onImgLoadComplete");
	
	var imageHeight = $(imageObj).height();

	var scrollHeight = document.getElementById('msgContent').scrollHeight;
	var scrollTop = document.getElementById('msgContent').scrollTop;
	var clientHeight = document.getElementById('msgContent').clientHeight;
	
	Message4Js.jsdebug("sh: " + scrollHeight + " st: " + scrollTop + " ch: " + clientHeight + " ih: " + imageHeight);
	
	if (scrollHeight-(scrollTop+clientHeight) <= imageHeight) {
		$("#msgContent").scrollTop(document.getElementById('msgContent').scrollHeight);
	}
	*/
 }

/**
 * 生成头像
 *
	 * obj:消息对象
 */
function makeAvatar(obj){
	var avatar = Message4Js.subscriptionId;
	var avatarClass = "subscription_avatar";
	
	if(obj.send == 1){
		avatar = Message4Js.uid;
		avatarClass = "user_avatar";
	}

	var msgAvatar = '<object type="application/x-qt-plugin" classid="'+avatarClass+'" width="34" height="34" user_id="'+avatar+'" />';
	return msgAvatar;
} 

/**
 * 生成文本消息的消息体
 *
	 * obj:消息对象
 */
function makeTextMsg(obj){
	
	var origin = 'receive';
	if(obj.send == 1){
		origin = 'send';
	}
	
	var content = Message4Js.convertFromPlainText(obj.content);

	content = replaceurl(content);

	var message = '';
	message += '<div class = "message" msgId = "'+obj.msgId+'">';
	message += '	<div class="msgtime">';
	message += '		<ul>';
	message += '			<li class="lineae"></li>';
	message += '			<li class="stime">';
	message += '				<a href="javascript:void(0)">';
	message += '					<span>' + obj.createTime + '</span>';
	message += '				</a>';
	message += '			</li>';
	message += '		</ul>';
	message += '	</div>';
	message += '	<div class="msgcontent msgcontent-' + origin + ' clearfix">';
	message += '		<i class = "' + origin + '">';
	message +=          	makeAvatar(obj);
	message += '		</i>';
	message += '		<div class="arrow-down arrow-down-'+ origin +'"></div>';
	message += '		<div class="content content-' + origin + '" >';
	message += 					content;
	message += '		</div>';
	message += '	</div>';
	message += '</div>';
	
	return message;
}


/**
 * 生成订阅号图文消息
 *
	 * obj:消息对象
 */
function makeImageTextMsg(obj)
{
	var message = '';
	
	if(obj.content == undefined || obj.content.length == 0){
		return message;
	}

	var origin = 'receive';

	message += '<div class = "message" msgId = "'+obj.msgId+'">';
	message += '	<div class="msgtime">';
	message += '		<ul>';
	message += '			<li class="lineae"></li>';
	message += '			<li class="stime">';
	message += '				<a href="javascript:void(0)">';
	message += '					<span>' + obj.createTime + '</span>';
	message += '				</a>';
	message += '			</li>';
	message += '		</ul>';
	message += '	</div>';
	message += '	<div class="msgcontent msgcontent-' + origin + ' clearfix">';
	message += '		<i class = "' + origin + '">';
	message +=          	makeAvatar(obj);
	message += '		</i>';
	message += '		<div class="arrow-down arrow-down-'+ origin +'"></div>';
	message += '		<div class="content content-' + origin + '">';

	var items = obj.content;
	if(items.length == 1){
		
		message += '<div class="news_message">';
        message += '   	<div class="news_title">';
        message += '    	<a href="javascript:void(0)" onclick="openTitle(' + items[0].id + ', ' + obj.id + ')" >' + items[0].title+ '</a>';
        message += '    </div>';
        message += '    <div class="news_thumb">';
        message += '    	<div class="news_media_thumb" src="' + items[0].picUrl + '" style="background-image:url(\'' + items[0].picUrl + '\'); background-color:#f1f1f6; background-repeat:no-repeat; background-position:center; -webkit-background-size:cover; height:150px;" />';
        message += '    </div>';
        message += '    <div class="news_summary">';
        message += '        ' + ((items[0].summary != null) ? items[0].summary : '')
        message += '    </div>';
        message += '</div>';
		
	} else {

		message += '<div class="news_message">'; 

		var len = items.length;

		for(var i = 0 ; i<items.length; i++){
			if(i == 0){
					message += '<div>';
					/*
					message += '	<div class="news_title">';
					message += '    	<a href="javascript:void(0)" onclick="openTitle(' + items[i].id + ', ' + obj.id + ')" >' + items[i].title + '</a>';
					message += '    </div>';
					*/
					message += '    <div class="news_thumb">';
					message += '    	<div class="news_media_thumb" src="' + items[i].picUrl + '" style="background-image:url(\'' + items[i].picUrl + '\'); background-color:#f1f1f6; background-repeat:no-repeat; background-position:center; -webkit-background-size:cover; height:150px;" >';
					message += '			<div class="news_thumb_title">';
					message += '    			<a href="javascript:void(0)" onclick="openTitle(' + items[i].id + ', ' + obj.id + ')" >' + items[i].title + '</a>';
					message += '    		</div>';
					message += '        </div>'
					message += '    </div>';
					message += '</div>';
			}else{

				if(i == len - 1){
					message += '<div class="news_item item_border_bottom">';
				}else{
					message += '<div class="news_item">';
				}
				
			    message += '	<img class="news_item_thumb" src="' + items[i].picUrl + '">';
			    message += '    <div class="news_item_title">';
			    message += '    	<a onclick="openTitle(' + items[i].id + ', ' + obj.id + ')"' +  'href="javascript:void(0);" >';
			    message += '        	' + items[i].title;
			    message += '        </a>';
			    message += '    </div>';
			    message += '</div>';
			}
		}
	}

	message += '		</div>';
	message += '	</div>';
	message += '</div>';

	return message;
}

/**
 * 生成订阅号图文消息
 *
	 * obj:消息对象
 */
function makeImageMsg(obj)
{
	var message = '';
	
	if(obj.content == undefined){
		return message;
	}

	var origin = 'receive';

	message += '<div class = "message" msgId = "'+obj.msgId+'">';
	message += '	<div class="msgtime">';
	message += '		<ul>';
	message += '			<li class="lineae"></li>';
	message += '			<li class="stime">';
	message += '				<a href="javascript:void(0)">';
	message += '					<span>' + obj.createTime + '</span>';
	message += '				</a>';
	message += '			</li>';
	message += '		</ul>';
	message += '	</div>';
	message += '	<div class="msgcontent msgcontent-' + origin + ' clearfix">';
	message += '		<i class = "' + origin + '">';
	message +=          	makeAvatar(obj);
	message += '		</i>';
	message += '		<div class="arrow-down arrow-down-'+ origin +'"></div>';
	message += '		<div class="content content-' + origin + '" >';

	var imageItem = obj.content;
	var imageWidth = 0;
	var imageHeight = 0;
	if (imageItem.width != undefined && imageItem.width != 0) {
		imageWidth = imageItem.width;
	}
	if (imageItem.height != undefined && imageItem.height != 0) {
		imageHeight = imageItem.height;
	}
	message += '<div class="image_message">';
	message += '    <div class="image_thumb">';
	message += '        <img class="image_media_thumb" src="' + imageItem.url + '" onload="onImgLoadComplete(this)" ';
	if (imageWidth != 0 && imageHeight != 0) {
		message += '      width="' + imageWidth +'" height="' + imageHeight + '"';
	}
	message += '        >';
	message += '    </div>';
	message += '</div>';

	message += '		</div>';
	message += '	</div>';
	message += '</div>';

	return message;
}

/**
 * 生成订阅号附件消息
 *
	 * obj:消息对象
 */
function makeAttachMsg(obj)
{
	var message = '';
	
	if(obj.content == undefined){
		return message;
	}

	var origin = 'receive';

	message += '<div class = "message" msgId = "'+obj.msgId+'">';
	message += '	<div class="msgtime">';
	message += '		<ul>';
	message += '			<li class="lineae"></li>';
	message += '			<li class="stime">';
	message += '				<a href="javascript:void(0)">';
	message += '					<span>' + obj.createTime + '</span>';
	message += '				</a>';
	message += '			</li>';
	message += '		</ul>';
	message += '	</div>';
	message += '	<div class="msgcontent msgcontent-' + origin + ' clearfix">';
	message += '		<i class = "' + origin + '">';
	message +=          	makeAvatar(obj);
	message += '		</i>';
	message += '		<div class="arrow-down arrow-down-'+ origin +'"></div>';
	message += '		<div class="content content-' + origin + '">';

	var attachItem = obj.content;
	
	// 确定附件类型
	var formatStr;
	var fileName = attachItem.name;
	var formatIndex = fileName.lastIndexOf(".");
	if (formatIndex == -1) {
		formatStr = "other";
	} else {
		var format = fileName.slice(formatIndex+1);
		format = format.toLowerCase();
		formatStr = format;
		if (!isConFormat(format)){
			formatStr = "other";
		}
	}
	
	// 文件名称长度处理
	var fileNameStr = fileName;
	if (fileNameStr.length > 30){
		fileNameStr = fileNameStr.substr(0,30)+"...";
	}
	
	message += '<div class="attach_message">'; 
	message += '<div class="attach_item">';		
	message += '	<img class="attach_item_thumb" src="' + 'qrc:/html/images/file_formats/'+formatStr+'.png' + '">';
	message += '    <a onclick="openAttach(\'' + attachItem.url + '\', \'' + fileName + '\')"' +  'href="javascript:void(0);" class="startdownload" >';
	message += '    </a>';
	message += '    <div class="attach_item_title" title="' + fileName + '" >';
	message += '		' + fileNameStr;
	message += '    </div>';
	message += '</div>';
	message += '</div>';

	message += '		</div>';
	message += '	</div>';
	message += '</div>';

	return message;
}

/**
 * 展示订阅号文本消息
 *
	 * obj:消息对象
 */
 function displayTextMsg(obj){

	if(obj == undefined){
		return false;
	}

	var origin = 'receive';
	if(obj.send == 1){
		origin = 'send';
	}
	
	var scrollHeight = document.getElementById('msgContent').scrollHeight;
	var scrollTop = document.getElementById('msgContent').scrollTop;
	var clientHeight = document.getElementById('msgContent').clientHeight;
	
	var message = makeTextMsg(obj);
	
	$(".chatBox_msgList").append(message);
	count++;
	
	if(origin == "receive"){					
		if(scrollTop+clientHeight >= scrollHeight){
			$("#msgContent").scrollTop(document.getElementById('msgContent').scrollHeight);
		}
	}else{
		$("#msgContent").scrollTop(document.getElementById('msgContent').scrollHeight);
	}

	return true;
}

/**
 * 展示订阅号图文消息
 *
	 * obj:消息对象
 */
function displayImageTextMsg(obj)
{
	if(obj == undefined){
		return false;
	}
	
	var scrollHeight = document.getElementById('msgContent').scrollHeight;
	var scrollTop = document.getElementById('msgContent').scrollTop;
	var clientHeight = document.getElementById('msgContent').clientHeight;
	
	var message = makeImageTextMsg(obj);
	$(".chatBox_msgList").append(message);
	count++;
	
	if(scrollTop+clientHeight >= scrollHeight){
		$("#msgContent").scrollTop(document.getElementById('msgContent').scrollHeight);
	}
	
	return true;
}

/**
 * 展示订阅号图片消息
 *
	 * obj:消息对象
 */
function displayImageMsg(obj)
{
	if(obj == undefined){
		return false;
	}
	
	var scrollHeight = document.getElementById('msgContent').scrollHeight;
	var scrollTop = document.getElementById('msgContent').scrollTop;
	var clientHeight = document.getElementById('msgContent').clientHeight;
	
	var message = makeImageMsg(obj);
	$(".chatBox_msgList").append(message);
	count++;
	
	if(scrollTop+clientHeight >= scrollHeight){
		$("#msgContent").scrollTop(document.getElementById('msgContent').scrollHeight);
	}
	
	return true;
}

/**
 * 展示订阅号附件消息
 *
	 * obj:消息对象
 */
function displayAttachMsg(obj)
{
	if(obj == undefined){
		return false;
	}
	
	var scrollHeight = document.getElementById('msgContent').scrollHeight;
	var scrollTop = document.getElementById('msgContent').scrollTop;
	var clientHeight = document.getElementById('msgContent').clientHeight;
	
	var message = makeAttachMsg(obj);
	$(".chatBox_msgList").append(message);
	count++;
	
	if(scrollTop+clientHeight >= scrollHeight){
		$("#msgContent").scrollTop(document.getElementById('msgContent').scrollHeight);
	}
	
	return true;
}

/**
 * 展示订阅号文本消息，显示在最前面
 *
	 * obj:消息对象
 */
function displayTextMsgAtTop(obj){

	if(obj == undefined){
		return false;
	}
	
	var origin = 'receive';
	if(obj.send == 1){
		origin = 'send';
	}
	
	var scrollHeight = document.getElementById('msgContent').scrollHeight;
	var scrollTop = document.getElementById('msgContent').scrollTop;
	var clientHeight = document.getElementById('msgContent').clientHeight;
	
	var message = makeTextMsg(obj);
	$(".chatBox_msgList").prepend(message);
	count++;
	
	if(origin == "receive"){					
		if(scrollTop+clientHeight >= scrollHeight){
			$("#msgContent").scrollTop(document.getElementById('msgContent').scrollHeight);
		}
	}else{
		$("#msgContent").scrollTop(document.getElementById('msgContent').scrollHeight);
	}

	return true;
}

/**
 * 展示订阅号图文消息，显示在最前面
 *
	 * obj:消息对象
 */
function displayImageTextMsgAtTop(obj)
{
	if(obj == undefined){
		return false;
	}
	
	var scrollHeight = document.getElementById('msgContent').scrollHeight;
	var scrollTop = document.getElementById('msgContent').scrollTop;
	var clientHeight = document.getElementById('msgContent').clientHeight;
	
	var message = makeImageTextMsg(obj);
	$(".chatBox_msgList").prepend(message);
	count++;
	
	if(scrollTop+clientHeight >= scrollHeight){
		$("#msgContent").scrollTop(document.getElementById('msgContent').scrollHeight);
	}
	
	return true;
}

/**
 * 展示订阅号图片消息，显示在最前面
 *
	 * obj:消息对象
 */
function displayImageMsgAtTop(obj)
{
	if(obj == undefined){
		return false;
	}
	
	var scrollHeight = document.getElementById('msgContent').scrollHeight;
	var scrollTop = document.getElementById('msgContent').scrollTop;
	var clientHeight = document.getElementById('msgContent').clientHeight;
	
	var message = makeImageMsg(obj);
	$(".chatBox_msgList").prepend(message);
	count++;
	
	if(scrollTop+clientHeight >= scrollHeight){
		$("#msgContent").scrollTop(document.getElementById('msgContent').scrollHeight);
	}
	
	return true;
}

/**
 * 展示订阅号附件消息，显示在最前面
 *
	 * obj:消息对象
 */
function displayAttachMsgAtTop(obj)
{
	if(obj == undefined){
		return false;
	}
	
	var scrollHeight = document.getElementById('msgContent').scrollHeight;
	var scrollTop = document.getElementById('msgContent').scrollTop;
	var clientHeight = document.getElementById('msgContent').clientHeight;
	
	var message = makeAttachMsg(obj);
	$(".chatBox_msgList").prepend(message);
	count++;
	
	if(scrollTop+clientHeight >= scrollHeight){
		$("#msgContent").scrollTop(document.getElementById('msgContent').scrollHeight);
	}
	
	return true;
}

/**
 * 展示订阅号消息
 *
	 * obj:消息对象
 */
function displaymsg(obj){

	if(obj == undefined){
		return false;
	}
	
	if(count >= maxMsgCount) {
		RemoveFirst();
	}
	
	/*
	Message4Js.jsdebug("displaymsg");
	*/
	
	if(obj.type == 2) {
		return displayImageTextMsg(obj);
	}
	else if(obj.type == 3) {
		return displayImageMsg(obj);
	}
	else if(obj.type == 4) {
		return displayAttachMsg(obj);
	}
	else {
		return displayTextMsg(obj);
	}
}

/**
 * 展示订阅号消息，显示在最前面
 *
	 * obj:消息对象
 */
function displaymsgAtTop(obj){

	if(obj == undefined){
		return false;
	}
	
	/*
	Message4Js.jsdebug("displaymsgAtTop");
	*/
	
	if(obj.type == 2) {
		return displayImageTextMsgAtTop(obj);
	}
	else if(obj.type == 3) {
		return displayImageMsgAtTop(obj);
	}
	else if(obj.type == 4) {
		return displayAttachMsgAtTop(obj);
	}
	else {
		return displayTextMsgAtTop(obj);
	}
}

/************************** 清屏 ************************************************/
// 移除所有消息
function RemoveAll() {
	$("div .chatBox_msgList").html("");
	count = 0;
}

// 清屏
function cleanup() {
	Message4Js.jsdebug("cleanup");
	RemoveAll();
}

// 界面已经显示好了
function initUIComplete() {
	Message4Js.jsdebug("initUIComplete");
	$("#msgContent").scrollTop(document.getElementById('msgContent').scrollHeight);
}

// 页面完成
function pageReady() {
	Message4Js.jsdebug("pageReady");
	Message4Js.setPageReady();
}

/************************** 更多消息 ************************************************/
// 显示更多消息提示栏
function showMoreMsgTip(){
	
	Message4Js.jsdebug("showMoreMsgTip");

	$("#moreBar").css("display","block");
}

// 隐藏更多消息提示栏
function closeMoreMsgTip(){

	Message4Js.jsdebug("closeMoreMsgTip");
	
	$("#moreBar").css("display","none");
}

// 加载更多消息开始
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

// 加载更多消息结束
function showMoreMsgFinish(){

	if($fdiv && $fdiv.length > 0){
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
