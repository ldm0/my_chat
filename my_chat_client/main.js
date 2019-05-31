const electron = require('electron');
const {ipcMain, app, BrowserWindow} = electron;
const kernel = require("./my_chat_client_kernel.node");

app.on('ready', main_window_create);
app.on('close', main_window_close);
app.on('window-all-closed', app.quit);

let main_window;

function main_window_create()
{
    console.log("create window");
    main_window = new BrowserWindow({
        webPreferences: {
            nodeIntegration : true,
        },
        width : 360,
        height : 640,
    });
    main_window.loadFile("./index.html");
    main_window.setMenu(null);
    main_window.on('close', main_window_close);
    //main_window.toggleDevTools();
    kernel.init();
    // set send message trigger
    ipcMain.on('send_msg', (event, msg) => {
        console.log('want to send message');
        kernel.send_msg(msg);
    });
    // set receive message service
    setInterval(() => {
        let msg_recv = kernel.get_msg_recv();
        if (msg_recv.length !== 0) {
            main_window.webContents.send('new_msg', msg_recv);
        }
    }, 100);
}

function main_window_close() 
{
    kernel.close();
    console.log("close window");
    main_window = null;
}
