FROM ubuntu

WORKDIR /app

COPY . /app

RUN apt-get update && apt-get install -y g++ make libcurl4-openssl-dev libpqxx-dev libpq-dev

RUN g++ -o app db.cpp -lcurl -lpqxx -lpq

CMD ["./app"]
