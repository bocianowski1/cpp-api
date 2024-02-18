#!/bin/bash

FOLDER="$1"

if [ -z "$FOLDER" ]; then
    echo "No folder specified."
    FOLDER="."
fi

cd $FOLDER

if [ ! -f .env ]; then
    echo "File .env does not exist."
    exit 1
fi

while IFS= read -r line; do
    if [[ $line == *"#"* ]]; then
        continue
    fi

    if [[ $line == *"="* ]]; then
        echo "export $line"
        export $line
    fi
    
done < .env