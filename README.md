# Wrapped C++

This is a project for learning C++. It adds the top songs from my _On Repeat_ playlist to my custom wrapped playlist. This project also uses a very simple HTTP Python server for handling the callback from Spotify and getting the code for later use.

## Run in Docker

`make run` or simply `make` — a shortcut for building and running the Dockerfile. This currently does not start the Python server.<br/>This must be done separately: `make serve`.

## Run locally

**Before running the code locally**
Export the required environment variables. Copy the output from `env.sh app`.

Install C++ Dependencies: `brew install libcurl`<br/>
Install Python Dependencies: `pip3 install -r requirements.txt`

Start Python server: `local-server`<br/>
Start C++ app: `make local`

> FireFox is used locally in development and Chrome is used in Docker in production.

## TODO:

- Don't add duplicate songs
- Remove duplicate code
- Deploy Python server
- Deploy C++ app
