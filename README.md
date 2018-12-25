# QTableViewPrinter
用于Qt qtableview的打印, 功能相对完善 功能有借鉴此处: https://github.com/T0ny0/Qt-Table-Printer.git, 感谢

一.开发环境
---
  * win10-64bit;
  * QT5.6.1-MINGW;
  
二.功能特点
---
  * 支持页眉, 页脚.
  * 支持标题,副标题
  * 支持多层表头, 表头合并
  * 打印宽度与qtableview列宽成比例, 调整qtableview即可调整打印宽度
  * 超出宽度的列自动换页, 超出高度的行自动换页
  * 支持打印可选字段, 选择什么字段需要自己来实现了, 没有设置隐藏字段不打印, 因为当前项目需要, 打印字段由客户自己选
  
三.示例截图
---
  * 初始qtableview界面.<br>
  ![image](./images/1.png)
  * 预览效果<br>
  ![image](./images/2.png)
  * 调整列宽<br>
  ![image](./images/3.png)
  * 再次预览, 因超出当前页最大列宽, 自动换页
  ![image](./images/4.png)
  ![image](./images/5.png)
  
四.LICENSE
---
 [BSD License](./LICENSE)
 
五.如有疑问, CONTACT ME
---
wechat: 'tonyonce'
