const url = "ws://192.168.4.1:1337/" // url para websocket
const dom = { // variável que contém todos os elementos utilizados aqui do DOM
    sliders: document.getElementById("sliders"),
    sliders_class: document.getElementsByClassName("slider"), 
    reset: document.getElementById("reset"),
    restart: document.getElementById("restart"),
    run: document.getElementById("run"),
    program: document.getElementById("program"),
    save: document.getElementById("save"),
    estado_conexao: document.getElementById("conexao-estado"),
    valores: document.getElementsByClassName("valores"),
    modo: document.getElementById("modo"),
    adiciona: document.getElementsByClassName("adiciona"),
    remove: document.getElementsByClassName("remove"),
    max_passo: document.getElementById("max"),
    passo: document.getElementById("passo")
}

const reset = [90,65,75,105];

function init() {
    // dom.sliders.style.pointerEvents = "none";
    wsConnect(url); // abre conexão com webSocket
    dom.sliders_class[0].addEventListener('input', (e) => { enviarSlider(1, e.target.value) }); // caso os sliders sejam mexidos,
    dom.sliders_class[1].addEventListener('input', (e) => { enviarSlider(2, e.target.value) }); // o programa irá enviar os dados
    dom.sliders_class[2].addEventListener('input', (e) => { enviarSlider(3, e.target.value) }); // automaticamente ao esp
    dom.sliders_class[3].addEventListener('input', (e) => { enviarSlider(4, e.target.value) });

    resetarValores();
    dom.passo.value, dom.max_passo.value = 0;

    dom.run.addEventListener('click', function(event){ // caso o botão run seja clicado
        event.preventDefault();
        enviar("run");
    })

    dom.save.addEventListener('click', function(event){
        event.preventDefault();
        enviar("sv");
    })

    dom.program.addEventListener('click', function(event){ // caso o botão program seja clicado
        event.preventDefault();
        enviar("pr");
    })

    dom.reset.addEventListener('click', function(event){ // função caso o      seja ativado
        if (dom.modo.innerHTML == "Program") {
            event.preventDefault(); // previne navegador de recarregar a página
            resetarValores();
            enviar("rs");
        }
    })
    
    dom.restart.addEventListener('click', function(event){
        event.preventDefault();
        resetarValores();
        dom.passo.value = 0;
        dom.max_passo.value = 0;
        enviar("rt");
    })

    for (var i = 0; i < dom.adiciona.length; i++){
        dom.adiciona[i].addEventListener('click', function(event){
            event.preventDefault();
            valor = parseInt(dom.sliders_class[event.target.id[1] - 1].value) + 1;
            dom.sliders_class[event.target.id[1] - 1].value = valor; 
            enviarSlider(parseInt(event.target.id[1]), valor)
        })
    }

    for (var i = 0; i < dom.remove.length; i++){
        dom.remove[i].addEventListener('click', function(event){
            event.preventDefault();
            valor = parseInt(dom.sliders_class[event.target.id[1] - 1].value) - 1;
            dom.sliders_class[event.target.id[1] - 1].value = valor; 
            enviarSlider(parseInt(event.target.id[1]), valor)
        })
    }
    
    for (var i = 0; i < dom.sliders_class.length; i++){
        dom.valores[i].innerHTML = dom.sliders_class[i].value;
    }

    dom.max_passo.addEventListener('input', function(event){
        event.preventDefault();
        if (dom.passo.value > dom.max_passo.value) {
            dom.passo.value = dom.max_passo.value;
        }
        enviar(';' + dom.max_passo.value + ';' + dom.passo.value);
    });

    dom.passo.addEventListener('input', function(event){
        event.preventDefault();
        if (dom.passo.value > dom.max_passo.value) {
            dom.passo.value = dom.max_passo.value;
        }
        enviar(';' + dom.max_passo.value + ';' + dom.passo.value);
    });
}

function enviarSlider(num, valor){
    dom.valores[num-1].innerHTML = valor;  // faz com que a label acima dos sliders, que indica o valor, mude de acordo com os sliders
    enviar(num + ";" + valor);
}

function wsConnect(url){
    ws = new WebSocket(url);
    ws.onopen = function(evt) {onOpen(evt)}; // caso seja aberto o websocket
    ws.onclose = function(evt) {onClose(evt)}; // se o websocket fechar
    ws.onmessage = function(evt) {onMessage(evt)}; // se uma mensagem ser recebida
    ws.onerror = function(evt) {onError(evt)}; // se um erro acontecer na comunicação
}

function onOpen(evt){
    console.log("Conectado");
    dom.estado_conexao.style.backgroundColor = "green"; // muda cor do "estado da conexão" na página
}

function onClose(evt){
    console.log("Desconectado");
    dom.estado_conexao.style.backgroundColor = "red"; // muda cor do "estado da conexão" na página
    // dom.sliders.style.pointerEvents = "none";
    setTimeout(function() { wsConnect(url)} , 1000); // aqui, basicamente, se a conexão fechar, o site tentará reconectar a cada 1 segundo
}

function onMessage(evt){
    console.log("Recebido: " + evt.data);
    payload = evt.data;
    if (payload == "run"){
        dom.sliders.style.pointerEvents = "none"; // faz com que os sliders não sejam alteráveis depois de se apertar Run
        dom.modo.innerHTML = "Run"; // muda o "modo de funcionamento" na página
        dom.restart.style.display = "none";
        dom.reset.style.display = "none";
        dom.save.style.display = "none";
    } else if (payload == "pr"){
        dom.sliders.style.pointerEvents = "auto"; // faz com que os sliders voltem a ser alteráveis depois de se apertar Program
        dom.modo.innerHTML = "Program";
        dom.restart.style.display = "inline";
        dom.reset.style.display = "inline";
        dom.save.style.display = "inline";
    } else {
        let str = payload.split(";");
        console.log(str);

        for (let i = 0; i < 4;i++){
            dom.valores[i].innerHTML = parseInt(str[i+2]);
            dom.sliders_class[i].value = parseInt(str[i+2]);
        }
        dom.max_passo.value = parseInt(str[1]);
        dom.passo.value = parseInt(str[0]);
    }
}

function onError(evt){
    console.log("ERRO: " + evt.data);
}

function enviar(msg) { // simplismente envia a mensagem
    console.log("Enviando: " + msg);
    ws.send(msg);
}

function resetarValores(){
    for (let i = 0; i < 4; i++){
        dom.sliders_class[i].value = reset[i];
        dom.valores[i].innerHTML = reset[i];;
    }
}

window.addEventListener("load", init, false); // faz com que a função init seja carregada assim que a página for carregada