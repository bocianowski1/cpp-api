FROM ubuntu

WORKDIR /app

COPY . /app

RUN apt-get update && apt-get install -y g++ make libcurl4-openssl-dev

RUN g++ -o http http.cpp -lcurl

CMD ["./http"]
