#!/bin/bash

if [ -f .env ]; then
    echo "File .env exists."
else
    echo "File .env does not exist."
    exit 1
fi

echo "Reading .env file..."
while IFS= read -r line; do
    if [[ $line == *"#"* ]]; then
        echo "Comment: $line"
        continue
    fi

    if [[ $line == *"="* ]]; then
        key=$(echo $line | cut -d'=' -f 1)
        echo "Key: $key"
        export $line
    fi
    
done < .env