
// from http://stackoverflow.com/questions/5515869/string-length-in-bytes-in-javascript
// using UTF8 strings I noticed that the javascript .length of a string returned less 
// characters than they actually were
var pendingSendMessages = [];
var ws = null;
var comTimeout = null;
var failedConnections = 0;

function byteLength(str) {
  // returns the byte length of an utf8 string
  var s = str.length;
  for (var i=str.length-1; i>=0; i--) {
    var code = str.charCodeAt(i);
    if (code > 0x7f && code <= 0x7ff) s++;
    else if (code > 0x7ff && code <= 0xffff) s+=2;
    if (code >= 0xDC00 && code <= 0xDFFF) i--; //trail surrogate
  }
  return s;
}

var paramPacketize = function (ps){
    var ret = '';
    for (var pkey in ps) {
        if( ret.length>0 )ret = ret + '|';
        var pstring = pkey+'='+ps[pkey];
        var pstring_length = byteLength(pstring);
        pstring = pstring_length+'|'+pstring;
        ret = ret + pstring;
    }
    return ret;
};

function openSocket(){
    try{
        ws = new WebSocket('ws://'+net_interface_ip+':'+wsport+'/');
        console.debug('opening websocket');
        ws.onopen = websocketOnOpen;
        ws.onmessage = websocketOnMessage;
        ws.onclose = websocketOnClose;
        ws.onerror = websocketOnError;
    }catch(ex){ws=false;alert('websocketnot supported or server unreachable');}
}

openSocket();

function websocketOnMessage (evt){
    var received_msg = evt.data;
    /*console.debug('Message is received:' + received_msg);*/
    var s = received_msg.split(',');
    var command = s[0];
    var index = received_msg.indexOf(',')+1;
    received_msg = received_msg.substr(index,received_msg.length-index);/*removing the command from the message*/
    index = received_msg.indexOf(',')+1;
    var content = received_msg.substr(index,received_msg.length-index);

    
    /*console.debug('content:' + content);*/

    if( command=='show_window' ){
        document.body.innerHTML = '<div id="loading" style="display: none;"><div id="loading-animation"></div></div>';
        document.body.innerHTML += decodeURIComponent(content);
    }else if( command=='update_widget'){
        var elem = document.getElementById(s[1]);
        var index = received_msg.indexOf(',')+1;
        elem.insertAdjacentHTML('afterend',decodeURIComponent(content));
        elem.parentElement.removeChild(elem);
    }else if( command=='insert_widget'){
        if( document.getElementById(s[1])==null ){
            /*the content contains an additional field that we have to remove*/
            index = content.indexOf(',')+1;
            content = content.substr(index,content.length-index);
            var elem = document.getElementById(s[2]);
            elem.innerHTML = elem.innerHTML + decodeURIComponent(content);
        }
    }else if( command=='javascript'){
        try{
            console.debug("executing js code: " + received_msg);
            eval(received_msg);
        }catch(e){console.debug(e.message);};
    }else if( command=='ack'){
        pendingSendMessages.shift() /*remove the oldest*/
        if(comTimeout!=null)clearTimeout(comTimeout);
    }else if( command=='ping'){
        ws.send('pong');
    }else{
        //console.debug(">>>>UNKNOWN MESSAGE TYPE: " + decodeURIComponent(received_msg));
    }
};

/*this uses websockets*/
var sendCallbackParam = function (widgetID,functionName,params /*a dictionary of name:value*/){
    var paramStr = '';
    if(params!=null) paramStr=paramPacketize(params);
    var message = encodeURIComponent(unescape('callback' + '/' + widgetID+'/'+functionName + '/' + paramStr));
    pendingSendMessages.push(message);
    if( pendingSendMessages.length < pending_messages_queue_length ){
        ws.send(message);
        if(comTimeout==null)
            comTimeout = setTimeout(checkTimeout, websocket_timeout_timer_ms);
    }else{
        console.debug('Renewing connection, ws.readyState when trying to send was: ' + ws.readyState)
        renewConnection();
    }
};

/*this uses websockets*/
var sendCallback = function (widgetID,functionName){
    sendCallbackParam(widgetID,functionName,null);
};

function renewConnection(){
    // ws.readyState:
    //A value of 0 indicates that the connection has not yet been established.
    //A value of 1 indicates that the connection is established and communication is possible.
    //A value of 2 indicates that the connection is going through the closing handshake.
    //A value of 3 indicates that the connection has been closed or could not be opened.
    if( ws.readyState == 1){
        try{
            ws.close();
        }catch(err){};
    }
    else if(ws.readyState == 0){
     // Don't do anything, just wait for the connection to be stablished
    }
    else{
        openSocket();
    }
};

function checkTimeout(){
    if(pendingSendMessages.length>0)
        renewConnection();    
};

function websocketOnClose(evt){
    /* websocket is closed. */
    console.debug('Connection is closed... event code: ' + evt.code + ', reason: ' + evt.reason);
    // Some explanation on this error: http://stackoverflow.com/questions/19304157/getting-the-reason-why-websockets-closed
    // In practice, on a unstable network (wifi with a lot of traffic for example) this error appears
    // Got it with Chrome saying:
    // WebSocket connection to 'ws://x.x.x.x:y/' failed: Could not decode a text frame as UTF-8.
    // WebSocket connection to 'ws://x.x.x.x:y/' failed: Invalid frame header

    try {
        document.getElementById("loading").style.display = '';
    } catch(err) {
        console.log('Error hiding loading overlay ' + err.message);
    }

    failedConnections += 1;

    console.debug('failed connections=' + failedConnections + ' queued messages=' + pendingSendMessages.length);

    if(failedConnections > 3) {

        // check if the server has been restarted - which would give it a new websocket address,
        // new state, and require a reload
        console.debug('Checking if GUI still up ' + location.href);

        var http = new XMLHttpRequest();
        http.open('HEAD', location.href);
        http.onreadystatechange = function() {
            if (http.status == 200) {
                // server is up but has a new websocket address, reload
                location.reload();
            }
        };
        http.send();

        failedConnections = 0;
    }

    if(evt.code == 1006){
        renewConnection();
    }

};

function websocketOnError(evt){
    /* websocket is closed. */
    /* alert('Websocket error...');*/
    console.debug('Websocket error... event code: ' + evt.code + ', reason: ' + evt.reason);
};

function websocketOnOpen(evt){
    if(ws.readyState == 1){
        ws.send('connected');

        try {
            document.getElementById("loading").style.display = 'none';
        } catch(err) {
            console.log('Error hiding loading overlay ' + err.message);
        }

        failedConnections = 0;

        while(pendingSendMessages.length>0){
            ws.send(pendingSendMessages.shift()); /*whithout checking ack*/
        }
    }
    else{
        console.debug('onopen fired but the socket readyState was not 1');
    }
};

function uploadFile(widgetID, eventSuccess, eventFail, eventData, file){
    var reader = new FileReader();
	reader.onloadend = function(evt) {
        if (evt.target.readyState == FileReader.DONE) { 
            console.debug(evt.target.result);
            var params={};
            params['file_name']=file.name;
            params['file_data']=evt.target.result;
            sendCallbackParam(widgetID, eventData, params);
		}
	};
    reader.readAsBinaryString(file.slice(0,file.size-1));
}