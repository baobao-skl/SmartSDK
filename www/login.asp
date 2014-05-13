<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8"/>
    <title>智能家居登录</title>
    <link href="css/css.css" rel="stylesheet" />
    <script type="text/javascript">
        function check()
        {
            if (loginForm.username.value == "")
            {
                alert("请输入用户名!");
                loginForm.username.focus();
                return false;
            }
            if (loginForm.password.value == "") {
                alert("请输入登录密码!");
                loginForm.password.focus();
                return false;
            }
            return true;
        }
    </script>
</head>
<body style="background-color:white">
    <div style="height:100px; position:absolute;left:0%;right:0%;top:0px;background-color:white">
    </div>
    <div style="height:420px; position:absolute;left:0%;right:0%;top:100px;background-color:brown">
      <table style="width:100%;height:100%;">
          <tr>
              <td style="width:60%;height:100%">
              </td>
              <td style="width:30%;height:100%">
                  <table style="width:100%;height:100%;">
                      <tr style="width:100%;height:5%">
                          <td></td>
                      </tr>
                      <tr style="width:100%;height:90%;background-color:white">
                          <td style="border-radius:5px">
                              <table style="width:100%;height:100%;">
                                  <tr>
                                      <td style="width:10%;height:100%"></td>
                                      <td style="width:80%;height:100%">
                                      <table style="height:100%;width:100%;">
                                       <form method="post" id="loginForm" name="loginForm" action="action/login">
                                          <tr style="height:20px;" valign="top"><td></td></tr>
                                          <tr style="height:30px"><td><label class="txt">智能家居用户登录</label><img src="icons\new.gif"/></td></tr>
                                          <tr style="height:25px"><td><hr style="line-height:2px; border-color:orange"/></td></tr>
                                          <tr style="height:25px"><td><label class="txt">请输入用户名:</label></td></tr>
                                          <tr style="height:40px"><td><input type="text" name="username" style="width:100%" class="txtBox"/></td></tr>
                                          <tr style="height:25px"><td><label class="txt">请输入密码:</label></td></tr>
                                          <tr style="height:40px"><td><input type="password" name="password" style="width:100%" class="txtBox"/></td></tr>
                                          <tr style="height:20px"><td></td></tr>
                                          <tr style="height:40px"><td><input type="submit" value="登录" onclick=" return check();" style="width:100%;height:40px;background-color:orange"/></td></tr>
                                          <tr style="height:auto"><td></td></tr> 
                                      </form>
                                         </table>
                                      </td>
                                      <td style="width:10%;height:100%"></td>
                                  </tr>
                              </table>
                          </td>
                      </tr>
                      <tr style="width:100%;height:5%">
                          <td></td>
                      </tr>
                  </table>

              </td>
              <td style="width:10%;height:100%">

              </td>
          </tr>
      </table>
            
    </div>

</body>
</html>
