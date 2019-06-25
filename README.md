## Reimplementação do Dropbox

Este repositório contém o trabalho da disciplina INF01151 (Sistemas Operacionais 2) da Universidade Federal do Rio Grande do Sul.

## Inicializando a aplicação

Para compilar o projeto deve ser utilizado o comando `make`.

Para iniciar um replica manager, deve ser utilizado `./dropboxRM <port> <id> <hostname1> <port1> ... <hostnameN> <portN>`, onde `<hostname1> <port1> ... <hostnameN> <portN>` é a lista de hostnames e portas dos outros RM, em ordem crescente de id. Ao rodar em um mesmo host, os RMs devem ser iniciados em diretórios diferentes e com portas também diferentes.

Exemplo de inicialização de 3 RMs:

RM 0:
`./dropboxRM 4000 0 localhost 4001 localhost 4002`

RM 1:
`./dropboxRM 4001 1 localhost 4000 localhost 4002`

RM 2:
`./dropboxRM 4002 2 localhost 4000 localhost 4001`

Inicialmente, o primário será o RM de maior id (o RM 2 no exemplo acima).

Para iniciar um cliente, use `./dropboxClient <username> <primary_hostname> <primary_port> <client_port>`. Rodando em uma mesma máquina, as portas e diretórios também devem ser diferentes.

Exemplo: conectando-se ao primário RM 2, que está rodando na porta 4002:
`./dropboxClient alice localhost 4002 4003`

No exemplo acima, a porta 4003 será usada para informar o cliente quando um novo primário for eleito.