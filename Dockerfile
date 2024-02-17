FROM ubuntu

WORKDIR /app
COPY . /app

RUN chmod +x env.sh

RUN apt-get update && apt-get install -y g++ make libcurl4-openssl-dev
RUN g++ -o app main.cpp -lcurl

CMD ["./app"]
