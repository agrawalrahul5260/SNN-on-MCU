/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "string.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include "cifar10_images.h"
#include "cifar_parameters.h"

#define INPUT_SIZE 32
#define THRESHOLD 1

#define LIF1_BETA 0.9
#define LIF2_BETA 0.9
#define LIF3_BETA 0.9
#define LIF4_BETA 0.9
#define LIF5_BETA 0.9

typedef struct {
    float membrane_potential;
    bool should_spike;
} LIFNeuron;

typedef struct {
    int in_channels;
    int out_channels;
    int kernel_size;
    int stride;
    int padding;
    const float (*weights)[CONV1_IN_CHANNELS][CONV1_KERNEL_SIZE][CONV1_KERNEL_SIZE];
} conv1;

typedef struct {
    int in_channels;
    int out_channels;
    int kernel_size;
    int stride;
    int padding;
    const float (*weights)[CONV2_IN_CHANNELS][CONV2_KERNEL_SIZE][CONV2_KERNEL_SIZE];
} conv2;

typedef struct {
    int in_channels;
    int out_channels;
    int kernel_size;
    int stride;
    int padding;
    const float (*weights)[CONV3_IN_CHANNELS][CONV3_KERNEL_SIZE][CONV3_KERNEL_SIZE];
} conv3;

typedef struct {
    const float (*weights)[FC1_OUT_FEATURES][FC1_IN_FEATURES];
} FullyConnectedLayer;

typedef struct {
    const float (*weights)[FC2_OUT_FEATURES][FC2_IN_FEATURES];
} FullyConnectedLayer2;

// Function to apply Leaky Integrate and Fire (LIF) neuron update
void update_neuron(LIFNeuron *neuron, float input_current, float beta, float threshold) {
    neuron->membrane_potential = (beta * neuron->membrane_potential + input_current);

    if (neuron->should_spike) {
        neuron->membrane_potential = 0;
        neuron->should_spike = false;
    } else if (neuron->membrane_potential >= threshold) {
        neuron->should_spike = true;
    }
}

// Function to perform 2D convolution
void conv1_2d(const float* input, float* output, const conv1* conv_layer, int input_size) {
    int output_size = (input_size - conv_layer->kernel_size + 2 * conv_layer->padding) / conv_layer->stride + 1;

    for (int oc = 0; oc < conv_layer->out_channels; ++oc) {
        for (int oh = 0; oh < output_size; ++oh) {
            for (int ow = 0; ow < output_size; ++ow) {
                float sum = 0;
                for (int ic = 0; ic < conv_layer->in_channels; ++ic) {
                    for (int kh = 0; kh < conv_layer->kernel_size; ++kh) {
                        for (int kw = 0; kw < conv_layer->kernel_size; ++kw) {
                            int ih = oh * conv_layer->stride + kh - conv_layer->padding;
                            int iw = ow * conv_layer->stride + kw - conv_layer->padding;
                            if (ih >= 0 && ih < input_size && iw >= 0 && iw < input_size) {
                                sum += input[ic * input_size * input_size + ih * input_size + iw] * conv_layer->weights[oc][ic][kh][kw];
                            }
                        }
                    }
                }
                output[oc * output_size * output_size + oh * output_size + ow] = sum;
            }
        }
    }
}

// Function to perform 2D convolution
void conv2_2d(float* input, float* output, const conv2* conv_layer, int input_size) {
    int output_size = (input_size - conv_layer->kernel_size + 2 * conv_layer->padding) / conv_layer->stride + 1;

    for (int oc = 0; oc < conv_layer->out_channels; ++oc) {
        for (int oh = 0; oh < output_size; ++oh) {
            for (int ow = 0; ow < output_size; ++ow) {
                float sum = 0;
                for (int ic = 0; ic < conv_layer->in_channels; ++ic) {
                    for (int kh = 0; kh < conv_layer->kernel_size; ++kh) {
                        for (int kw = 0; kw < conv_layer->kernel_size; ++kw) {
                            int ih = oh * conv_layer->stride + kh - conv_layer->padding;
                            int iw = ow * conv_layer->stride + kw - conv_layer->padding;
                            if (ih >= 0 && ih < input_size && iw >= 0 && iw < input_size) {
                                sum += input[ic * input_size * input_size + ih * input_size + iw] * conv_layer->weights[oc][ic][kh][kw];
                            }
                        }
                    }
                }
                output[oc * output_size * output_size + oh * output_size + ow] = sum;
            }
        }
    }
}

// Function to perform 2D convolution
void conv3_2d(float* input, float* output, const conv3* conv_layer, int input_size) {
    int output_size = (input_size - conv_layer->kernel_size + 2 * conv_layer->padding) / conv_layer->stride + 1;

    for (int oc = 0; oc < conv_layer->out_channels; ++oc) {
        for (int oh = 0; oh < output_size; ++oh) {
            for (int ow = 0; ow < output_size; ++ow) {
                float sum = 0;
                for (int ic = 0; ic < conv_layer->in_channels; ++ic) {
                    for (int kh = 0; kh < conv_layer->kernel_size; ++kh) {
                        for (int kw = 0; kw < conv_layer->kernel_size; ++kw) {
                            int ih = oh * conv_layer->stride + kh - conv_layer->padding;
                            int iw = ow * conv_layer->stride + kw - conv_layer->padding;
                            if (ih >= 0 && ih < input_size && iw >= 0 && iw < input_size) {
                                sum += input[ic * input_size * input_size + ih * input_size + iw] * conv_layer->weights[oc][ic][kh][kw];
                            }
                        }
                    }
                }
                output[oc * output_size * output_size + oh * output_size + ow] = sum;
            }
        }
    }
}

// Function to perform 2D max pooling
void maxpool2d(float* input, float* output, int in_channels, int input_size, int kernel_size, int stride) {
    int output_size = (input_size - kernel_size) / stride + 1;

    for (int ic = 0; ic < in_channels; ++ic) {
        for (int oh = 0; oh < output_size; ++oh) {
            for (int ow = 0; ow < output_size; ++ow) {
                int ih = oh * stride;
                int iw = ow * stride;
                float max_value = input[ic * input_size * input_size + ih * input_size + iw];
                for (int kh = 0; kh < kernel_size; ++kh) {
                    for (int kw = 0; kw < kernel_size; ++kw) {
                        int nih = ih + kh;
                        int niw = iw + kw;
                        if (nih < input_size && niw < input_size) {
                            float value = input[ic * input_size * input_size + nih * input_size + niw];
                            if (value > max_value) {
                                max_value = value;
                            }
                        }
                    }
                }
                output[ic * output_size * output_size + oh * output_size + ow] = max_value;
            }
        }
    }
}

// Function to perform inference
int inference(const float input_image[3][INPUT_SIZE][INPUT_SIZE], conv1* conv1, conv2* conv2, conv3* conv3, FullyConnectedLayer* fc_layer1, FullyConnectedLayer2* fc_layer2) {
    // Step 1: Convolutional Layer 1
    float conv1_output[CONV1_OUT_CHANNELS][INPUT_SIZE][INPUT_SIZE] = {0};
    LIFNeuron lif1_neurons[CONV1_OUT_CHANNELS * INPUT_SIZE * INPUT_SIZE] = {0};
    conv1_2d(&input_image[0][0][0], &conv1_output[0][0][0], conv1, INPUT_SIZE);

    // Step 2: Apply LIF neurons to conv1 output
    for (int i = 0; i < CONV1_OUT_CHANNELS * INPUT_SIZE * INPUT_SIZE; i++) {
        update_neuron(&lif1_neurons[i], conv1_output[0][0][i], LIF1_BETA, THRESHOLD);
        conv1_output[0][0][i] = lif1_neurons[i].should_spike ? 1.0 : 0.0;
    }

    // Step 3: Max Pooling 1
    int pool1_output_size = INPUT_SIZE / 2;
    float pool1_output[CONV1_OUT_CHANNELS][INPUT_SIZE / 2][INPUT_SIZE / 2] = {0};
    maxpool2d(&conv1_output[0][0][0], &pool1_output[0][0][0], CONV1_OUT_CHANNELS, INPUT_SIZE, 2, 2);

    // Step 4: Convolutional Layer 2
    int conv2_input_size = pool1_output_size;
    float conv2_output[CONV2_OUT_CHANNELS][INPUT_SIZE / 2][INPUT_SIZE / 2] = {0};
    LIFNeuron lif2_neurons[CONV2_OUT_CHANNELS * (INPUT_SIZE / 2) * (INPUT_SIZE / 2)] = {0};
    conv2_2d(&pool1_output[0][0][0], &conv2_output[0][0][0], conv2, conv2_input_size);

    // Step 5: Apply LIF neurons to conv2 output
    for (int i = 0; i < CONV2_OUT_CHANNELS * conv2_input_size * conv2_input_size; i++) {
        update_neuron(&lif2_neurons[i], conv2_output[0][0][i], LIF2_BETA, THRESHOLD);
        conv2_output[0][0][i] = lif2_neurons[i].should_spike ? 1.0 : 0.0;
    }

    // Step 6: Max Pooling 2
    int pool2_output_size = conv2_input_size / 2;
    float pool2_output[CONV2_OUT_CHANNELS][(INPUT_SIZE / 2)/2][(INPUT_SIZE / 2)/2] = {0};
    maxpool2d(&conv2_output[0][0][0], &pool2_output[0][0][0], CONV2_OUT_CHANNELS, conv2_input_size, 2, 2);

    // Step 7: Convolutional Layer 3
    int conv3_input_size = pool2_output_size;
    float conv3_output[CONV3_OUT_CHANNELS][(INPUT_SIZE / 2)/2][(INPUT_SIZE / 2)/2] = {0};
    LIFNeuron lif3_neurons[CONV3_OUT_CHANNELS * (INPUT_SIZE / 2)/2 * (INPUT_SIZE / 2)/2] = {0};
    conv3_2d(&pool2_output[0][0][0], &conv3_output[0][0][0], conv3, conv3_input_size);

    // Step 8: Apply LIF neurons to conv3 output
    for (int i = 0; i < CONV3_OUT_CHANNELS * conv3_input_size * conv3_input_size; i++) {
        update_neuron(&lif3_neurons[i], conv3_output[0][0][i], LIF3_BETA, THRESHOLD);
        conv3_output[0][0][i] = lif3_neurons[i].should_spike ? 1.0 : 0.0;
    }

    // Step 9: Max Pooling 3
    int pool3_output_size = conv3_input_size / 2;
    float pool3_output[CONV3_OUT_CHANNELS][(INPUT_SIZE / 2)/2/2][(INPUT_SIZE / 2)/2/2] = {0};
    maxpool2d(&conv3_output[0][0][0], &pool3_output[0][0][0], CONV3_OUT_CHANNELS, conv3_input_size, 2, 2);

    // Step 10: Fully Connected Layer 1
    int fc1_input_size = CONV3_OUT_CHANNELS * pool3_output_size * pool3_output_size;
    float fc1_input[fc1_input_size];
    for (int c = 0; c < CONV3_OUT_CHANNELS; c++) {
        for (int h = 0; h < pool3_output_size; h++) {
            for (int w = 0; w < pool3_output_size; w++) {
                fc1_input[c * pool3_output_size * pool3_output_size + h * pool3_output_size + w] = pool3_output[c][h][w];
            }
        }
    }

    float fc1_output[FC1_OUT_FEATURES] = {0};
    LIFNeuron lif4_neurons[FC1_OUT_FEATURES] = {0};

    for (int i = 0; i < FC1_OUT_FEATURES; i++) {
        float sum = 0;
        for (int j = 0; j < fc1_input_size; j++) {
            sum += (fc1_weights[i][j]) * fc1_input[j];
        }
        update_neuron(&lif4_neurons[i], sum, LIF4_BETA, THRESHOLD);
        fc1_output[i] = lif4_neurons[i].should_spike ? 1.0 : 0.0;
    }

    // Step 11: Fully Connected Layer 2
    float fc2_output[FC2_OUT_FEATURES] = {0};
    LIFNeuron lif5_neurons[FC2_OUT_FEATURES] = {0};

    for (int i = 0; i < FC2_OUT_FEATURES; i++) {
        float sum = 0;
        for (int j = 0; j < FC1_OUT_FEATURES; j++) {
            sum += (fc2_weights[i][j]) * fc1_output[j];
        }
        update_neuron(&lif5_neurons[i], sum, LIF5_BETA, THRESHOLD);
        fc2_output[i] = lif5_neurons[i].should_spike ? 1.0 : 0.0;
    }

    // Determine the predicted class (the index of the maximum value in fc2_output)
    int predicted_class = 0;
    float max_value = fc2_output[0];
    for (int i = 1; i < FC2_OUT_FEATURES; i++) {
        if (fc2_output[i] > max_value) {
            max_value = fc2_output[i];
            predicted_class = i;
        }
    }

    return predicted_class;
}
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
#if defined ( __ICCARM__ ) /*!< IAR Compiler */
#pragma location=0x30000000
ETH_DMADescTypeDef  DMARxDscrTab[ETH_RX_DESC_CNT]; /* Ethernet Rx DMA Descriptors */
#pragma location=0x30000200
ETH_DMADescTypeDef  DMATxDscrTab[ETH_TX_DESC_CNT]; /* Ethernet Tx DMA Descriptors */

#elif defined ( __CC_ARM )  /* MDK ARM Compiler */

__attribute__((at(0x30000000))) ETH_DMADescTypeDef  DMARxDscrTab[ETH_RX_DESC_CNT]; /* Ethernet Rx DMA Descriptors */
__attribute__((at(0x30000200))) ETH_DMADescTypeDef  DMATxDscrTab[ETH_TX_DESC_CNT]; /* Ethernet Tx DMA Descriptors */

#elif defined ( __GNUC__ ) /* GNU Compiler */

ETH_DMADescTypeDef DMARxDscrTab[ETH_RX_DESC_CNT] __attribute__((section(".RxDecripSection"))); /* Ethernet Rx DMA Descriptors */
ETH_DMADescTypeDef DMATxDscrTab[ETH_TX_DESC_CNT] __attribute__((section(".TxDecripSection")));   /* Ethernet Tx DMA Descriptors */
#endif

ETH_TxPacketConfig TxConfig;

ADC_HandleTypeDef hadc1;
ADC_HandleTypeDef hadc2;
ADC_HandleTypeDef hadc3;

DAC_HandleTypeDef hdac1;

ETH_HandleTypeDef heth;

FDCAN_HandleTypeDef hfdcan1;
FDCAN_HandleTypeDef hfdcan2;

I2C_HandleTypeDef hi2c4;

LTDC_HandleTypeDef hltdc;

OSPI_HandleTypeDef hospi1;
OSPI_HandleTypeDef hospi2;

SAI_HandleTypeDef hsai_BlockA1;
SAI_HandleTypeDef hsai_BlockB1;

SD_HandleTypeDef hsd1;

TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim4;
TIM_HandleTypeDef htim5;

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart3;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void PeriphCommonClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM1_Init(void);
static void MX_USART1_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
    // Initialize convolutional layers
    conv1 conv1 = {CONV1_IN_CHANNELS, CONV1_OUT_CHANNELS, CONV1_KERNEL_SIZE, CONV1_STRIDE, CONV1_PADDING, (const float (*)[CONV1_IN_CHANNELS][CONV1_KERNEL_SIZE][CONV1_KERNEL_SIZE])conv1_weights};
    conv2 conv2 = {CONV2_IN_CHANNELS, CONV2_OUT_CHANNELS, CONV2_KERNEL_SIZE, CONV2_STRIDE, CONV2_PADDING, (const float (*)[CONV2_IN_CHANNELS][CONV2_KERNEL_SIZE][CONV2_KERNEL_SIZE])conv2_weights};
    conv3 conv3 = {CONV3_IN_CHANNELS, CONV3_OUT_CHANNELS, CONV3_KERNEL_SIZE, CONV3_STRIDE, CONV3_PADDING, (const float (*)[CONV3_IN_CHANNELS][CONV3_KERNEL_SIZE][CONV3_KERNEL_SIZE])conv3_weights};

    // Initialize fully connected layer
    FullyConnectedLayer fc_layer1 = {(const float (*)[FC1_OUT_FEATURES][FC1_IN_FEATURES])fc1_weights};
    FullyConnectedLayer2 fc_layer2 = {(const float (*)[FC2_OUT_FEATURES][FC2_IN_FEATURES])fc2_weights};

    int predicted_class;
    uint32_t timestamp0;
    uint32_t timestamp1;
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

/* Configure the peripherals common clocks */
  PeriphCommonClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_TIM1_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  timestamp0 = htim1.Instance->CNT;
	  predicted_class = inference(cifar10_images[0], &conv1, &conv2, &conv3, &fc_layer1, &fc_layer2);
	  timestamp1 = htim1.Instance->CNT;

	  HAL_Delay(500);
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Supply configuration update enable
  */
  HAL_PWREx_ConfigSupply(PWR_DIRECT_SMPS_SUPPLY);

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI48|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 5;
  RCC_OscInitStruct.PLL.PLLN = 110;
  RCC_OscInitStruct.PLL.PLLP = 1;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_2;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief Peripherals Common Clock Configuration
  * @retval None
  */
void PeriphCommonClock_Config(void)
{
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Initializes the peripherals clock
  */
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInitStruct.PLL2.PLL2M = 5;
  PeriphClkInitStruct.PLL2.PLL2N = 80;
  PeriphClkInitStruct.PLL2.PLL2P = 5;
  PeriphClkInitStruct.PLL2.PLL2Q = 2;
  PeriphClkInitStruct.PLL2.PLL2R = 2;
  PeriphClkInitStruct.PLL2.PLL2RGE = RCC_PLL2VCIRANGE_2;
  PeriphClkInitStruct.PLL2.PLL2VCOSEL = RCC_PLL2VCOWIDE;
  PeriphClkInitStruct.PLL2.PLL2FRACN = 0;
  PeriphClkInitStruct.AdcClockSelection = RCC_ADCCLKSOURCE_PLL2;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
}


/**
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};
  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 0;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 65535;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_PWM_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterOutputTrigger2 = TIM_TRGO2_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_4) != HAL_OK)
  {
    Error_Handler();
  }
  sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
  sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
  sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
  sBreakDeadTimeConfig.DeadTime = 0;
  sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
  sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
  sBreakDeadTimeConfig.BreakFilter = 0;
  sBreakDeadTimeConfig.Break2State = TIM_BREAK2_DISABLE;
  sBreakDeadTimeConfig.Break2Polarity = TIM_BREAK2POLARITY_HIGH;
  sBreakDeadTimeConfig.Break2Filter = 0;
  sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
  if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */
  HAL_TIM_MspPostInit(&htim1);

}


/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart1, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart1, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}


/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, Detectn_Pin|LCD_DISP_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOE, ARD_D8_Pin|STMOD_17_Pin|STMOD_19_Pin|STMOD_18_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOG, LCD_BL_CTRL_Pin|ARD_D7_Pin|MEMS_LED_Pin|ARD_D4_Pin
                          |ARD_D2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, USER_LED2_Pin|USER_LED1_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(STMOD_20_GPIO_Port, STMOD_20_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOH, LCD_RST_Pin|USB_FS_PWR_EN_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : Detectn_Pin LCD_DISP_Pin */
  GPIO_InitStruct.Pin = Detectn_Pin|LCD_DISP_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pins : SAI4_D2_Pin SAI4_CK2_Pin */
  GPIO_InitStruct.Pin = SAI4_D2_Pin|SAI4_CK2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF10_SAI4;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pins : ARD_D8_Pin STMOD_17_Pin STMOD_19_Pin STMOD_18_Pin */
  GPIO_InitStruct.Pin = ARD_D8_Pin|STMOD_17_Pin|STMOD_19_Pin|STMOD_18_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pins : USB_FS_OVCR_Pin CTP_INT_Pin */
  GPIO_InitStruct.Pin = USB_FS_OVCR_Pin|CTP_INT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

  /*Configure GPIO pin : Blue_button_B2_used_for_wakeup_Pin */
  GPIO_InitStruct.Pin = Blue_button_B2_used_for_wakeup_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(Blue_button_B2_used_for_wakeup_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : USB_FS_ID_Pin */
  GPIO_InitStruct.Pin = USB_FS_ID_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF10_OTG1_HS;
  HAL_GPIO_Init(USB_FS_ID_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : LCD_BL_CTRL_Pin ARD_D7_Pin MEMS_LED_Pin ARD_D4_Pin
                           ARD_D2_Pin */
  GPIO_InitStruct.Pin = LCD_BL_CTRL_Pin|ARD_D7_Pin|MEMS_LED_Pin|ARD_D4_Pin
                          |ARD_D2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

  /*Configure GPIO pin : USB_FS_VBUS_Pin */
  GPIO_InitStruct.Pin = USB_FS_VBUS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(USB_FS_VBUS_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : uSD_Detect_Pin */
  GPIO_InitStruct.Pin = uSD_Detect_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(uSD_Detect_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : USER_LED2_Pin USER_LED1_Pin */
  GPIO_InitStruct.Pin = USER_LED2_Pin|USER_LED1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : SPI5_MISO_Pin */
  GPIO_InitStruct.Pin = SPI5_MISO_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF5_SPI5;
  HAL_GPIO_Init(SPI5_MISO_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : SPI5_MOSI_Pin */
  GPIO_InitStruct.Pin = SPI5_MOSI_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF5_SPI5;
  HAL_GPIO_Init(SPI5_MOSI_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : Audio_Int_Pin */
  GPIO_InitStruct.Pin = Audio_Int_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(Audio_Int_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : STMOD_20_Pin */
  GPIO_InitStruct.Pin = STMOD_20_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(STMOD_20_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : STMOD_11_INT_Pin */
  GPIO_InitStruct.Pin = STMOD_11_INT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(STMOD_11_INT_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : LCD_RST_Pin USB_FS_PWR_EN_Pin */
  GPIO_InitStruct.Pin = LCD_RST_Pin|USB_FS_PWR_EN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOH, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */