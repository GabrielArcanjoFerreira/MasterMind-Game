#include <iostream>
#include <fstream>
#include <string>
#include <speechapi_cxx.h>
#include <locale>
#include <random>
#include <Windows.h>

/*************************************************************************************************

    Nome do projeto: MasterMind - Game
    Autor: Gabriel Arcanjo dos Santos Ferreira
    Data: 08/12/2020
    Versão: 1.0.0

**************************************************************************************************/

// Constantes para utilização das cores
#define VERMELHO 12
#define VERDE 10
#define AZUL 9
#define AMARELO 6
#define BRANCO 7

using namespace std;
using namespace Microsoft::CognitiveServices::Speech;
using namespace Microsoft::CognitiveServices::Speech::Audio;

HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE); // Handle para o recurso do atributo que guarda a cor do texto do console 

// Inicialição das variáveis para comunicação com o serviço de reconhecimento de voz do Azure
auto autenticacao = SpeechConfig::FromSubscription("d8b69092e26740ca9780f363c2ce9248", "brazilsouth");
auto audio_config = AudioConfig::FromDefaultMicrophoneInput();
auto requisicao_textofala = SpeechSynthesizer::FromConfig(autenticacao);
auto requisicao_falatexto = SpeechRecognizer::FromConfig(autenticacao, audio_config);


class ReconhecimentoFala {
    /*
    Descrição: Classe principal para acesso do serviço de reconhecimento de voz
    */
    private:
        string idioma;
        string voz;
        string chave;
        string regiao;

    public:
        ReconhecimentoFala(string idioma, string voz, string chave, string regiao) {
            /*
            Descrição: Construtor da classe, nele são atualizados os atributos do recurso no Azure.
            Entradas: Idioma da fala (String), Voz do locutor (String), Chave do recurso (String) e Região do recurso (String).
            */
            autenticacao = SpeechConfig::FromSubscription(chave, regiao);
            autenticacao->SetSpeechRecognitionLanguage(idioma);
            autenticacao->SetSpeechSynthesisLanguage(idioma);
            autenticacao->SetSpeechSynthesisVoiceName(voz);
            requisicao_textofala = SpeechSynthesizer::FromConfig(autenticacao);
            requisicao_falatexto = SpeechRecognizer::FromConfig(autenticacao, audio_config);
        }

        string get_idioma() {
            return this->idioma;
        }

        string get_voz() {
            return this->voz;
        }

        string get_chave() {
            return this->chave;
        }
        
        string get_regiao() {
            return this->regiao;
        }

        void texto_para_fala(string texto) {
            /*
            Descrição: Método responsável por transcrever texto em fala.
            Entradas: O texto a ser transcrito (String).
            */
            requisicao_textofala->SpeakTextAsync(texto).get();
        }

        string fala_para_texto() {
            /*
            Descrição: Método responsável por transcrever fala em texto.
            Saídas: Texto transcrito a partir da fala (String).
            */
            auto resultado = requisicao_falatexto->RecognizeOnceAsync().get();
            return resultado->Text;
        }
};


class MasterMind {
    /*
    Descrição: Classe principal para execução do jogo.
    */

    private:
        string palpite_cores[4];
        string sequencia_cores[4];
        string cores_default[4] = { "Vermelho", "Verde", "Azul", "Amarelo" };
        string indicador;
        int cor_indicador;
        int num_tentativas;
        bool qlf_contido;
        bool qlf_venceu;

    public:
        MasterMind(int num_tentativas){
            this->num_tentativas = num_tentativas;
        }

        void gerar_seq_cores() {
            /*
            Descrição: Método responsável pela geração da sequência de cores.
            */
            srand(time(0));

            // Loop para geração de cada cor
            for (int cor = 0; cor <= 3; cor++) {
                this->sequencia_cores[cor] = this->cores_default[rand() % 4];
            }
        }

        int get_num_tentativas() {
            return this->num_tentativas;
        }

        int define_cor(string cor, int posicao) {
            /*
            Descrição: Método responsável por identificar a cor da letra do console durante a entrada do usuário.
            Entradas: Cor falada (String), Posição da cor falada (Inteiro).
            Saídas: Cor do console (Inteiro).
            */

            if (cor == "Vermelho." || cor == "Vermelha.") {
                // Para o vermelho é feita dupla veirifcação, para avaliação da palavra no feminino e masculino
                palpite_cores[posicao] = "Vermelho"; // O vetor do palpite é montado simultâneamente com a definição da cor do console
                return VERMELHO;
            }
            else if (cor == "Verde.") {
                palpite_cores[posicao] = "Verde";
                return VERDE;
            }            
            else if (cor == "Azul.") {
                palpite_cores[posicao] = "Azul";
                return AZUL;
            }
            else {
                // Por padrão se não for identificada a cor nas acima, será definida a cor amarela
                palpite_cores[posicao] = "Amarelo";
                return AMARELO;
            }
        }
        
        void corrigir(int posicao) {
            /*
            Descrição: Método responsável por avaliar o indicador correto para a cor em cada posição.
            Entradas: Posição da cor a ser avalida (Inteiro).
            */

            // Verifica se a cor está na posição correta
            if (this->palpite_cores[posicao] == this->sequencia_cores[posicao]) {
                this->indicador = "CP";
                this->cor_indicador = VERDE;
            }
            // Se não, realiza a busca dela internamente no vetor de cores da sequencia
            else {
                this->qlf_venceu = false;  // Já marca o o qualificador como palpite incorreto
                this->qlf_contido = false;

                for (int pos = 0; pos <= 3; pos++){
                    if (pos != posicao && palpite_cores[posicao] == sequencia_cores[pos]) {
                        qlf_contido = true;
                    }
                }

                if (qlf_contido == true) {
                    this->indicador = "C";
                    this->cor_indicador = VERMELHO;
                }
                else {
                    this->indicador = " ";
                    this->cor_indicador = BRANCO;
                }
            }
        }

        bool imprimir_correcao() {
            /*
            Descrição: Método responsável por imprimir a correção na tela para o usuário.
            Saídas: Qualificador que indica vitória ou não (Booleano).
            */

            this->qlf_venceu = true; // Qualificador que indica se o palpite foi correto ou não

            // Sempre feita a chamada da função de correção passando a posição da cor avaliada
            corrigir(0);
            SetConsoleTextAttribute(hConsole, this->cor_indicador);
            cout << "    [" << this->indicador << "]";
            corrigir(1);
            SetConsoleTextAttribute(hConsole, this->cor_indicador);
            cout << " [" << this->indicador << "]";
            corrigir(2);
            SetConsoleTextAttribute(hConsole, this->cor_indicador);
            cout << " [" << this->indicador << "]";
            corrigir(3);
            SetConsoleTextAttribute(hConsole, this->cor_indicador);
            cout << " [" << this->indicador << "]" << endl;

            return this->qlf_venceu;
        }

        void imprimir_tutorial(ReconhecimentoFala locutor) {
            /*
            Descrição: Método responsável por imprimir a tela de tutorial para o usuário.
            Entradas: O objeto que representa a conexão com o serviço com o reconhecimento de voz do Azure.
            */

            system("cls");
            SetConsoleTextAttribute(hConsole, VERMELHO);
            cout << "\n\n\t\t\t\t" << "MasterMind - Game" << "\n\n";
            SetConsoleTextAttribute(hConsole, BRANCO);
            cout << "       Neste jogo seu objetivo e adivinhar uma sequencia de cores aleatorias" << endl;
            locutor.texto_para_fala("Neste jogo seu objetivo é adivinhar uma sequência de cores aleatórias");
            cout << "       Voce fara isto por meio de comandos de voz durante suas tentativas!" << endl;
            locutor.texto_para_fala("Você fará isto por meio de comandos de voz durante suas tentativas!");
            cout << "       A cada rodada voce sempre deve informar 4 cores. Observe o exemplo:" << endl;
            locutor.texto_para_fala("A cada rodada você sempre deverá informar 4 cores. Observe o exemplo:");

            SetConsoleTextAttribute(hConsole, BRANCO);
            cout << "\n\t\t\t     Usuario          Correcao";
            SetConsoleTextAttribute(hConsole, AMARELO);
            cout << "\n\t\t\t [x]";
            SetConsoleTextAttribute(hConsole, VERMELHO);
            cout << " [x]";
            SetConsoleTextAttribute(hConsole, VERDE);
            cout << " [x]";
            SetConsoleTextAttribute(hConsole, AZUL);
            cout << " [x]";

            SetConsoleTextAttribute(hConsole, VERMELHO);
            cout << "  [C]";
            SetConsoleTextAttribute(hConsole, VERMELHO);
            cout << " [C]";
            SetConsoleTextAttribute(hConsole, BRANCO);
            cout << " [ ]";
            SetConsoleTextAttribute(hConsole, VERDE);
            cout << " [CP]" << endl;

            SetConsoleTextAttribute(hConsole, BRANCO);
            cout << "\n       A correcao e feita atraves dos indicadores 'C' e 'CP'" << endl;
            locutor.texto_para_fala("A correção é feita através dos indicadores 'C' e 'CP'");
            SetConsoleTextAttribute(hConsole, VERMELHO);
            cout << "\n       [C]: Cor correta e posicao incorreta";
            locutor.texto_para_fala("O indicador C informa que a cor existe na sequência, porém a mesma está na posição errada!");
            SetConsoleTextAttribute(hConsole, VERDE);
            cout << "\n       [CP]: Cor e posicao correta";
            locutor.texto_para_fala("O indicador CP informa que a cor está na posição correta!");
            SetConsoleTextAttribute(hConsole, BRANCO);
            cout << "\n       [ ]: Cor nao presente na sequencia, deve ser descartada" << endl;
            locutor.texto_para_fala("Ausência de indicador significa que a cor não está presente na sequência, logo deve ser descartada!");
        }
};


int main()
{
    /*
    Descrição: Função principal do jogo. Faz chamada de todos os demais objetos.
    */

    string fala_usuario;
    bool qlf_venceu;

    setlocale(LC_ALL, "");

    // Configurações iniciais reconhecimento de fala
    ReconhecimentoFala reconhecimento_fala("pt-BR", "pt-BR-Daniel", "d8b69092e26740ca9780f363c2ce9248", "brazilsouth");

    // Configurações iniciais do jogo;
    MasterMind game(8);

    // Configura tamanho da fonte do console
    CONSOLE_FONT_INFOEX cfi;
    cfi.cbSize = sizeof(cfi);
    cfi.nFont = 0;
    cfi.dwFontSize.X = 0;
    cfi.dwFontSize.Y = 32;
    SetCurrentConsoleFontEx(GetStdHandle(STD_OUTPUT_HANDLE), FALSE, &cfi);
    

    // Trativa se houver alguma excessão inesperada da comunicação com o Azure
    try {
        // Tela inicial do game
        SetConsoleTextAttribute(hConsole, VERMELHO);
        cout << "\n\n\t\t\t\t" << "MasterMind - Game" << "\n\n";
        reconhecimento_fala.texto_para_fala("BEM VINDO AO MASTER MIND GAME");

        SetConsoleTextAttribute(hConsole, BRANCO);
        cout << "\n\n\t\t\t" << "Pular tutorial? (Fale Sim ou Nao)" << "\n\n";
        fala_usuario = reconhecimento_fala.fala_para_texto();

        // Verifica se o usuário deseja pular o tutorial
        if (fala_usuario != "Sim.") {
            game.imprimir_tutorial(reconhecimento_fala);

            cout << "\n\n\t\t\t  " << "Iniciar? (Fale Sim ou Nao)" << "\n";
            fala_usuario = reconhecimento_fala.fala_para_texto();

            if (fala_usuario != "Sim.") {
                system("cls");
                return 0;
            }
        }

        do {
            // Inicio do jogo
            system("cls");
            cout << "\n\n\t     " << "Fale apenas as cores Vermelho, Verde, Azul ou Amarelo" << "\n\n";

            game.gerar_seq_cores(); // Geração da sequência de cores aleatória

            for (int tentativa = 1; tentativa <= game.get_num_tentativas(); tentativa++) {
                // Inicio e print de cada tentativa feita pelo usuário
                SetConsoleTextAttribute(hConsole, VERMELHO);
                cout << "  Tentativa " << tentativa << ":";
                SetConsoleTextAttribute(hConsole, game.define_cor(reconhecimento_fala.fala_para_texto(), 0));
                cout << "\t\        [x]";
                SetConsoleTextAttribute(hConsole, game.define_cor(reconhecimento_fala.fala_para_texto(), 1));
                cout << " [x]";
                SetConsoleTextAttribute(hConsole, game.define_cor(reconhecimento_fala.fala_para_texto(), 2));
                cout << " [x]";
                SetConsoleTextAttribute(hConsole, game.define_cor(reconhecimento_fala.fala_para_texto(), 3));
                cout << " [x]";

                qlf_venceu = game.imprimir_correcao();

                // Verificar se a sequência foi a certa
                if (qlf_venceu == true) {
                    cout << "\n\n\t\t\t\t   " << "Parabens!" << "\n\n";
                    reconhecimento_fala.texto_para_fala("Você venceu!");
                    break;
                }
            }

            // Caso derrota finaliza o jogo.
            if (qlf_venceu == false) {
                SetConsoleTextAttribute(hConsole, VERMELHO);
                cout << "\n\n\t\t\t\t   " << "Fim de jogo!" << "\n\n";
                reconhecimento_fala.texto_para_fala("Não desanime!");
            }

            // Verifica se encerra o jogo ou tenta novamente
            SetConsoleTextAttribute(hConsole, BRANCO);
            cout << "\t\t      " << "Jogar novamente? (Fale Sim ou Nao)" << "\n";
            fala_usuario = reconhecimento_fala.fala_para_texto();

        } while (fala_usuario == "Sim.");

    }
    catch (exception error)
    {
        cout << error.what();
    }
    
    system("cls");
    return 0;
}
