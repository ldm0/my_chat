const {ipcRenderer} = require('electron');

ipcRenderer.on('new_msg', (event, arg) => {
    show_new_msg("别人", arg);
});

var msgs;
var msg_input_words;
var msg_input_send_button;

window.onload = () => {
    msgs = document.getElementById("msgs");
    msg_input_words = document.getElementById("msg_input_words");
    msg_input_send_button = document.getElementById("msg_input_send_button");
    msg_input_send_button.onclick = () => {
        ipcRenderer.send('send_msg', msg_input_words.value);
        show_new_msg("自己", msg_input_words.value);
        msg_input_words.value = '';
    };
};

function show_new_msg(who, msg) {
    if (who === "自己") {       // if string is said by myself
        str = '<p class="msg_myself"><span>' + who + ': ' + msg + '</span></p>';
    } else {
        str = '<p class="msg_others"><span>' + who + ': ' + msg + '</span></p>';
    }
    msgs.innerHTML = msgs.innerHTML + str;
}
