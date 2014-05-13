
function control(ActionName) {
    var xmlhttp;
    if (window.XMLHttpRequest) {// code for IE7+, Firefox, Chrome, Opera, Safari
        xmlhttp = new XMLHttpRequest();
    } else {// code for IE6, IE5
        xmlhttp = new ActiveXObject("Microsoft.XMLHTTP");
    }
    xmlhttp.open("GET", "/action/music?action="+ActionName.trim()+"&time="+new Date(), true);
    xmlhttp.send();
}

function SetWinHeight(obj) {
    var win = obj;
    if (document.getElementById) {
        if (win && !window.opera) {
            if (win.contentDocument && win.contentDocument.body.offsetHeight)
                win.height = win.contentDocument.body.offsetHeight - 50;
            else if (win.Document && win.Document.body.scrollHeight)
                win.height = win.Document.body.scrollHeight -50;
        }
    }
}