# Wrapped C++

This is a project for learning C++. It adds the top songs from my _On Repeat_ playlist to my custom wrapped playlist. This project also uses a very simple HTTP Go server for handling the callback from Spotify and getting the code for later use.

## Run in Docker

`make run` or simply `make` - a shortcut for building and running the Dockerfile. This currently does not start the Go server. This must be done separately: `make server`.

## TODO:

- Automate login to spotify
- Dont add duplicates
- Dockerize Go server
- Deploy
