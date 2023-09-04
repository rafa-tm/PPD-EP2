import numpy as np
import matplotlib.pyplot as plt
import argparse
import os

parser = argparse.ArgumentParser(description='How to use this program')
parser.add_argument("--folder", type=str, default="wavefield", help="Path to the folder containing wavefield files")
args = parser.parse_args()

# Verifique se a pasta de destino existe
output_folder = 'plots'
if not os.path.exists(output_folder):
    os.makedirs(output_folder)

# Listar todos os arquivos na pasta wavefield que começam com "wavefield_"
wavefield_files = [file for file in os.listdir(args.folder) if file.startswith("wavefield_")]

# Iterar sobre os arquivos e gerar os gráficos correspondentes
for wavefield_file in wavefield_files:
    # Construir o caminho completo para o arquivo
    file_path = os.path.join(args.folder, wavefield_file)
    
    # Ler os dados do arquivo
    input_data = np.loadtxt(file_path)

    # Processar os dados e gerar o gráfico
    plt.imshow(input_data)
    
    # Extrair o número do arquivo, por exemplo, "wavefield_100" se torna "100"
    file_number = wavefield_file.split("_")[1]
    
    # Salvar o gráfico com um nome correspondente
    output_filename = os.path.join(output_folder, "wavefield_{}.png".format(file_number))
    plt.savefig(output_filename, format='png')
    
    print("Plot saved in", output_filename)

print("All plots generated and saved.")
