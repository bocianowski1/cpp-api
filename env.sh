#!/bin/bash

if [ -f .env ]; then
    echo "File .env exists."
else
    echo "File .env does not exist."
    exit 1
fi

while IFS= read -r line; do
    if [[ $line == *"#"* ]]; then
        continue
    fi

    if [[ $line == *"="* ]]; then
        export $line
    fi
    
done < .env