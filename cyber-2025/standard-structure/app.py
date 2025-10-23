# first and last name: Riccardo Palazzi
# student id: 187432
# path: $HOME/nome-progetto/app.py

import argparse
import os
import sys

# aggiungi import mancanti del template

def walk(basepath, ...):
    for filename in os.listdir(basepath):
        path = os.path.join(basepath, filename)
        if os.path.isfile(path):
            # logica per i file
            pass
        elif os.path.isdir(path):
            # ricorsione
            walk(path, ...)

def main():

    #1. SETUP ARGPARSE 
    
    parser = argparse.ArgumentParser(description="Descrizione del programma")
    parser.add_argument(
        "--arg1", 
        type=str, 
        required=True, 
        help="..."
        )
    parser.add_argument(
        "--arg2", 
        type=int, 
        required=True, 
        help="..."
        )
    args = parser.parse_args()

    #2. VALIDAZIONE INPUT
    # Sempre nello stesso ordine:

    # a) Verifica del path assoluto
    if not os.path.isabs(args.path):
        print(f"error: {args.path} does not exist", file=sys.stderr)
        sys.exit(1)

    # b) Verifica esistenza del path
    if not os.path.exists(args.path):
        print(f"error: {args.path} does not exist", file=sys.stderr)
        sys.exit(1)
    
    # c) Verifica del tipo di path (file o directory)
    if not os.path.isdir(args.path):
        print(f"error: {args.path} is not a directory", file=sys.stderr)
        sys.exit(1)

    # d) Verifica range numerici (numeri interi o positivi) 
    if args.size <= 0:
        print(f"error: --size must be a positive integer.", file=sys.stderr)
        sys.exit(1)

    #3. SETUP DIRECTORY / FILE OUTPUT
    output_dir = os.path.expanduser("~/nome-directory")
    os.makedirs(output_dir, exist_ok=True)

    #4. LOGICA PRINCIPALE 

    if __name__ == "__main__":
        main()

