entrar = document.getElementById("entrar");
email = document.getElementById("email");
password = document.getElementById("password");
err_email = document.getElementById("email-erro");
err_senha = document.getElementById("senha-erro");

err_email.style.display = "none";
err_senha.style.display = "none";

entrar.addEventListener("click", function(event){
    event.preventDefault();
    if (validaEmail() == true && validaSenha() == true){
        location.href = "prog.html"
    }
})

function validaEmail(){
    if(email.value == ""){
        err_email.style.display = "block";
        return false;
    }
    err_email.style.display = "none";
    return true;
}

function validaSenha(){
    if(password.value == ""){
        err_senha.style.display = "block";
        return false;
    }
    err_senha.style.display = "none";
    return true;
}