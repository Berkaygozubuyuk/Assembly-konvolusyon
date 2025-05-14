#define _CRT_SECURE_NO_DEPRECATE
#include <stdio.h>
#include <stdlib.h>

// Function to print a matrix stored in a 1D array
void print_matrix(unsigned* matrix, unsigned rows, unsigned cols, FILE* file);
// Function to read matrix from a file
void read_matrix(const char* filename, unsigned** matrix, unsigned* rows, unsigned* cols);
// Function to read kernel from a file
void read_kernel(const char* filename, unsigned** kernel, unsigned* k);
// Function to write output matrix to a file
void write_output(const char* filename, unsigned* output, unsigned rows, unsigned cols);
// Initialize output as zeros.
void initialize_output(unsigned*, unsigned, unsigned);

int main() {

    unsigned n, m, k;  // n = rows of matrix, m = cols of matrix, k = kernel size
    // Dynamically allocate memory for matrix, kernel, and output
    unsigned* matrix = NULL;  // Input matrix
    unsigned* kernel = NULL;  // Kernel size 3x3
    unsigned* output = NULL;  // Max size of output matrix

    char matrix_filename[30];
    char kernel_filename[30];

    // Read the file names
    printf("Enter matrix filename: ");
    scanf("%s", matrix_filename);
    printf("Enter kernel filename: ");
    scanf("%s", kernel_filename);


    // Read matrix and kernel from files
    read_matrix(matrix_filename, &matrix, &n, &m);  // Read matrix from file
    read_kernel(kernel_filename, &kernel, &k);      // Read kernel from file

    // For simplicity we say: padding = 0, stride = 1
    // With this setting we can calculate the output size
    unsigned output_rows = n - k + 1;
    unsigned output_cols = m - k + 1;
    output = (unsigned*)malloc(output_rows * output_cols * sizeof(unsigned));
    initialize_output(output, output_rows, output_cols);

    // Print the input matrix and kernel
    printf("Input Matrix: ");
    print_matrix(matrix, n, m, stdout);

    printf("\nKernel: ");
    print_matrix(kernel, k, k, stdout);

    /******************* KODUN BU KISMINDAN SONRASINDA DEĞİŞİKLİK YAPABİLİRSİNİZ - ÖNCEKİ KISIMLARI DEĞİŞTİRMEYİN *******************/

    // Assembly kod bloğu içinde kullanacağınız değişkenleri burada tanımlayabilirsiniz. ---------------------->
    // Aşağıdaki değişkenleri kullanmak zorunda değilsiniz. İsterseniz değişiklik yapabilirsiniz.

    unsigned indeks = 1; // kernel satirinin sonuna gelip gelmedigimizi kontrol etmek icin kullaniyoruz
    unsigned jump = (m - k) * 4; // matrisin bir alt satirina gecmek icin
    //x ve y satir sonuna geldigimde ilerlemeyi saglamak icin.
    unsigned x = (k - 1) * 4;   
    unsigned y = m - k + 1;
    unsigned output_size = output_rows * output_cols;
    unsigned kernel_size = k * k;
    // Assembly dilinde 2d konvolüsyon işlemini aşağıdaki blokta yazınız ----->
    __asm {

        MOV EDI, matrix //Matris baslangic adresi
        MOV ESI, kernel //Kernel baslangic adresi
        MOV EBX, output //Cikti baslangic adresi
        MOV ECX, output_size
        MOV EAX, 0
        PUSH EAX
        JMP start


        main_loop :
            MOV EAX, 1
            MOV indeks, EAX
            ADD EDI, 4          //input indisim
            ADD EBX, 4          //bir sonraki output indisine gecmek icin
            XOR EAX, EAX        
            XOR EDX, EDX
            MOV ESI, kernel

            //matris icinde gezerken satirin sonuna gelinirse kontrolu
            POP EAX
            INC EAX
            PUSH EAX
            MOV EDX, y
            DIV DL
            CMP AH, 0
            JNE start
            ADD EDI, x

        start :
            XOR EAX, EAX
            XOR EDX, EDX
            PUSH EDI
            PUSH ECX
            MOV ECX, kernel_size

            // indislerin carpilip output icine eklendigi dongu
        inner_loop :
            MOV AX, [EDI]       // matristeki degeri aliyoruz
            MOV DX, [ESI]       // kerneldeki degeri aliyoruz
            MUL DX              // matris ve kerneldeki ilgili degerin carpimi
            ADD[EBX], EAX
            XOR EAX, EAX
            XOR EDX, EDX
            ADD EDI, 4          // her eleman 4 bayt kapladigi icin
            ADD ESI, 4

            // kernel satirinin sonuna geldik mi kontrolu
            MOV EDX, k          //kernel satir uzunlugunu EDX icine atiyorum
            MOV EAX, indeks
            DIV DL
            CMP AH, 0           // eger kalan sifirsa satirin sonundayiz demektir
            JNE devam
            ADD EDI, jump       //  bir alt satira gecmek icin jump degeri ekliyoruz

        devam :
            INC indeks
            LOOP inner_loop
            // inner_loop sonu

            POP ECX
            POP EDI
    LOOP    main_loop

            POP EAX
    }

    /******************* KODUN BU KISMINDAN ÖNCESİNDE DEĞİŞİKLİK YAPABİLİRSİNİZ - SONRAKİ KISIMLARI DEĞİŞTİRMEYİN *******************/


    // Write result to output file
    write_output("./output.txt", output, output_rows, output_cols);

    // Print result
    printf("\nOutput matrix after convolution: ");
    print_matrix(output, output_rows, output_cols, stdout);

    // Free allocated memory
    free(matrix);
    free(kernel);
    free(output);

    return 0;
}

void print_matrix(unsigned* matrix, unsigned rows, unsigned cols, FILE* file) {
    if (file == stdout) {
        printf("(%ux%u)\n", rows, cols);
    }
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            fprintf(file, "%u ", matrix[i * cols + j]);
        }
        fprintf(file, "\n");
    }
}

void read_matrix(const char* filename, unsigned** matrix, unsigned* rows, unsigned* cols) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("Error opening file %s\n", filename);
        exit(1);
    }

    // Read dimensions
    fscanf(file, "%u %u", rows, cols);
    *matrix = (unsigned*)malloc(((*rows) * (*cols)) * sizeof(unsigned));

    // Read matrix elements
    for (int i = 0; i < (*rows); i++) {
        for (int j = 0; j < (*cols); j++) {
            fscanf(file, "%u", &(*matrix)[i * (*cols) + j]);
        }
    }

    fclose(file);
}

void read_kernel(const char* filename, unsigned** kernel, unsigned* k) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("Error opening file %s\n", filename);
        exit(1);
    }

    // Read kernel size
    fscanf(file, "%u", k);
    *kernel = (unsigned*)malloc((*k) * (*k) * sizeof(unsigned));

    // Read kernel elements
    for (int i = 0; i < (*k); i++) {
        for (int j = 0; j < (*k); j++) {
            fscanf(file, "%u", &(*kernel)[i * (*k) + j]);
        }
    }

    fclose(file);
}

void write_output(const char* filename, unsigned* output, unsigned rows, unsigned cols) {
    FILE* file = fopen(filename, "w");
    if (!file) {
        printf("Error opening file %s\n", filename);
        exit(1);
    }

    // Write dimensions of the output matrix
    fprintf(file, "%u %u\n", rows, cols);

    // Write output matrix elements
    print_matrix(output, rows, cols, file);

    fclose(file);
}

void initialize_output(unsigned* output, unsigned output_rows, unsigned output_cols) {
    int i;
    for (i = 0; i < output_cols * output_rows; i++)
        output[i] = 0;
    
}

