#include <stdio.h>
#include <string.h>
#include "esp_system.h"
#include "esp_log.h"
#include "esp_console.h"
#include "driver/uart.h"
#include "esp_vfs_dev.h"
#include "linenoise/linenoise.h"
#include "argtable3/argtable3.h"

// Definindo a UART2
#define UART_NUM UART_NUM_2
#define UART_TX_PIN 17
#define UART_RX_PIN 16
#define UART_BAUD_RATE 115200

static const char* TAG = "uart_console";

// Função de inicialização da UART2
void init_uart()
{
    const uart_config_t uart_config = {
        .baud_rate = UART_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };

    // Configurando a UART
    uart_param_config(UART_NUM, &uart_config);
    uart_set_pin(UART_NUM, UART_TX_PIN, UART_RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_driver_install(UART_NUM, 1024, 0, 0, NULL, 0);

    // Vinculando a UART2 ao VFS para permitir acesso ao console
    esp_vfs_dev_uart_use_driver(UART_NUM);
}

// Função de inicialização do console
void init_console()
{
    // Configuração do console com a UART
    esp_vfs_dev_uart_set_rx_line_endings(ESP_LINE_ENDINGS_CR);
    esp_vfs_dev_uart_set_tx_line_endings(ESP_LINE_ENDINGS_CRLF);

    // Inicializando o console
    esp_console_config_t console_config = {
        .max_cmdline_args = 8,
        .max_cmdline_length = 256,
        .hint_color = atoi(LOG_COLOR_CYAN)
    };
    ESP_ERROR_CHECK(esp_console_init(&console_config));

    // Habilitando histórico de comandos
    linenoiseSetMultiLine(1);
    linenoiseHistorySetMaxLen(100);
}

// Função para registrar um comando básico de exemplo
static int hello_cmd(int argc, char **argv)
{
    printf("Hello, UART Console!\n");
    return 0;
}

void register_commands()
{
    const esp_console_cmd_t hello_cmd_def = {
        .command = "hello",
        .help = "Print 'Hello, UART Console!'",
        .hint = NULL,
        .func = &hello_cmd,
    };
    ESP_ERROR_CHECK(esp_console_cmd_register(&hello_cmd_def));
}

void app_main(void)
{
    // Inicializando a UART2 e o console
    init_uart();
    init_console();

    // Registrando comandos
    register_commands();

    // Loop principal do terminal
    while (true) {
        char* line = linenoise("esp32> ");
        if (line == NULL) { // Verifica se a linha é válida
            continue;
        }

        // Adiciona o comando ao histórico
        linenoiseHistoryAdd(line);

        // Executa o comando digitado
        int ret = esp_console_run(line, NULL);
        if (ret == ESP_ERR_NOT_FOUND) {
            printf("Comando não encontrado\n");
        } else if (ret == ESP_ERR_INVALID_ARG) {
            // Erro ao passar argumentos inválidos para o comando
            printf("Erro de argumento\n");
        }

        linenoiseFree(line); // Libera a memória da linha de comando
    }
}
