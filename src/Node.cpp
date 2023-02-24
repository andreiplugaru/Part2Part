#include <map>
#include <algorithm>
#include <sys/stat.h>
#include "Node.h"
#include "Network.h"
#include <sqlite3.h>
#include <cstring>
#include <fcntl.h>

Result Node::requestInfoFromSuperNode(in_addr_t Ip, in_port_t port){
    ConnectToSuperNodeRequest request;
    int sd;
    Result result = Network::makeRequest(Ip, port, RequestType::GetNeighbourInfo, sd);
    if(result== Success)
    {
        NextSuperNodeResponse nextSuperNodeResponse;

        Network::receive(sd,nextSuperNodeResponse);
        if(this->scannedSuperNodes->find(Ip) != this->scannedSuperNodes->end())
        {
            if(nextSuperNodeResponse.available) {
                this->scannedSuperNodes->find(Ip)->second = nextSuperNodeResponse.foundRatio;
            }
            else
                this->scannedSuperNodes->find(Ip)->second = -1;
        }
        if(this->scannedSuperNodes->find(nextSuperNodeResponse.Nextip) == this->scannedSuperNodes->end())
        {

                   /* char addressBuffer[INET_ADDRSTRLEN];
                    inet_ntop(AF_INET, &nextSuperNodeResponse.Nextip, addressBuffer, INET_ADDRSTRLEN);
            printf("next ip recived: %s", addressBuffer);*/
                    if(nextSuperNodeResponse.available) {
                        superNodesPrevs.insert(std::pair<in_addr_t, in_addr_t>(nextSuperNodeResponse.Nextip, Ip));
                        this->scannedSuperNodes->insert(std::pair<in_addr_t, int>(nextSuperNodeResponse.Nextip, 0));
                    }
                    else {
                        superNodesPrevs.insert(std::pair<in_addr_t, in_addr_t>(nextSuperNodeResponse.Nextip, Ip));
                        this->scannedSuperNodes->insert(std::pair<in_addr_t, int>(nextSuperNodeResponse.Nextip, -1));
                    }

               /* }
                else {
                    printf("alone\n");
                    if(nextSuperNodeResponse.available)
                        this->scannedSuperNodes->insert(std::pair<in_addr_t, int>(Ip, 0));
                    else
                        this->scannedSuperNodes->insert(std::pair<in_addr_t, int>(Ip, -1));

                }*/

          //  if(!nextSuperNodeResponse.isAlone)
       //   printf("recurisve call\n");
            char ipStr[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &nextSuperNodeResponse.Nextip, ipStr, INET_ADDRSTRLEN);
             result = requestInfoFromSuperNode(nextSuperNodeResponse.Nextip, htons(atoi("2908")));
        }
    }

  /*  else if(result  == Failure)
    {
        perror ("Unknown host error().\n");
        close(sd);
        return Failure;
    }*/
    close(sd);
    return result;
}
NextSuperNodeResponse Node::makeNewSuperNode() {
    int sd;
    NextSuperNodeResponse response;
    auto max = std::max_element(this->scannedSuperNodes->begin(), this->scannedSuperNodes->end());
    Result result = Network::makeRequest(superNodesPrevs.find(max->first)->second, htons(atoi("2908")), RequestType::UpdateNextNodeNeighbour, sd);
    if (result == Success) {
        if (write (sd,&this->ip,sizeof(in_addr_t)) <= 0)
        {
            perror ("Write error().\n");
        }
        Network::receive(sd,response);
        response.result = Success;
        return response;
    }
    response.result = Failure;
    return response;
}
bool Node::hasAvailableSuperNodes()
{
    auto max = getMaxSuperNode();
    return max.second != -1;
}
std::pair<in_addr_t, int> Node::getMaxSuperNode()
{
    std::pair<in_addr_t, int> max;
    max.second = -1;
    for (auto i : *this->scannedSuperNodes) {
    if(i.second > max.second)
        max = i;
    }
    return max;
}
Result Node::connectToSuperNode() {
    auto max = getMaxSuperNode();
    if (max.second != -1) {
        int sd;
            Result result = Network::makeRequest(max.first, htons(atoi("2908")), RequestType::ConnectToSuperNode, sd);
            if (result == Success) {
                AcceptSuperNodeResponse response;
                Network::receive(sd,response);
                this->shouldBeRedundantSuperNode = response.shouldBeRedundantSuperNode;
                close(sd);
                this->ipSuperNode = max.first;
                return Success;
            }
            else
                perror("Error when connecting to the supernode!");
        }
    exit(0);
}
Node::Node() {
this->ip = 0;
this->ip = getIp();
this->scannedSuperNodes = new std::unordered_map<in_addr_t, int>();
}
void Node::addFiles(std::string fileName) {
    struct stat st;
    if(stat(fileName.c_str(), &st) == -1) {
        printf("File does not exist!");
        return;
    }
    insertFile(fileName, "", st.st_size);
}
static int callback(void *NotUsed, int argc, char **argv, char **azColName) {
    int i;
    for(i = 0; i<argc; i++) {
        printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }
    printf("\n");
    return 0;
}
void Node::insertFile(std::string fileName, std::string type, int size)
{
    sqlite3 *db;
    char *zErrMsg = 0;
    int rc;
  //  char *sql;
    rc = sqlite3_open("node.db", &db);

    if( rc ) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        return;
    } else {
        //printf(stderr, "Opened database successfully\n");
    }
/* Create SQL statement */
std::string name =   basename (fileName.c_str());
std::string sql = "INSERT INTO FILES (ID, NAME, SIZE) "  \
         "VALUES (NULL,'"+name+"', '"+std::to_string(size)+"'); ";

    /* Execute SQL statement */
    rc = sqlite3_exec(db, sql.c_str(), callback, 0, &zErrMsg);

    if( rc != SQLITE_OK ){
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    } else {
       // fprintf(stdout, "Records created successfully\n");
    }
    sqlite3_close(db);

}
void Node::initDB() {
    char *zErrMsg = 0;
    int rc;
    std::string sql;
    sqlite3 *db;

    /* Open database */
    rc = sqlite3_open("node.db", &db);

    if( rc ) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        return;
    } else {
        //fprintf(stdout, "Opened database successfully\n");
    }

    /* Create SQL statement */

    sql = "DROP TABLE FILES";


    /* Execute SQL statement */
    rc = sqlite3_exec(db, sql.c_str(), callback, 0, &zErrMsg);

    if( rc != SQLITE_OK ){
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    } else {
        //  fprintf(stdout, "Table created successfully\n");
    }


    sql = "CREATE TABLE FILES("  \
      "ID INTEGER PRIMARY KEY AUTOINCREMENT," \
      "NAME           TEXT    NOT NULL," \
      "SIZE            INTEGER   ," \
      "TYPE          CHAR(50) );";
    /* Execute SQL statement */
    rc = sqlite3_exec(db, sql.c_str(), callback, 0, &zErrMsg);

    if( rc != SQLITE_OK ){
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    } else {
        //  fprintf(stdout, "Table created successfully\n");
    }
    sqlite3_close(db);
}
int getFileSize(FileRequest fileRequest)
{
    Result result = Failure;
    sqlite3* db;
    sqlite3_stmt* stmt;
    std::string sql;

    // create sql statement string
    // if _id is not 0, search for id, otherwise print all IDs
    // this can also be achieved with the default sqlite3_bind* utilities
    sql = "select * from FILES Where name ='"+std::string(fileRequest.fileName)+"';";

    //the resulting sql statement
    //printf("sql: %s\n", sql.c_str());

    //get link to database object
    if(sqlite3_open("node.db", &db) != SQLITE_OK) {
        printf("ERROR: can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return Failure;
    }

    // compile sql statement to binary
    if(sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL) != SQLITE_OK) {
        printf("ERROR: while compiling sql: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        sqlite3_finalize(stmt);
        return Failure;
    }

    // execute sql statement, and while there are rows returned, print ID
    int ret_code = 0;
    while((ret_code = sqlite3_step(stmt)) == SQLITE_ROW) {
        int size = sqlite3_column_int(stmt, 2);
        sqlite3_close(db);
        sqlite3_finalize(stmt);

        return size;
    }
    if(ret_code != SQLITE_DONE) {
        //this error handling could be done better, but it works
        printf("ERROR: while performing sql: %s\n", sqlite3_errmsg(db));
        printf("ret_code = %d\n", ret_code);
        sqlite3_finalize(stmt);
        sqlite3_close(db);(db);

        return Success;
    }
   // sqlite3_finalize(stmt);
    sqlite3_close(db);
    return Failure;
}
Result Node::checkFileExists(FileRequest fileRequest) {
   Result result = Failure;
    sqlite3* db;
    sqlite3_stmt* stmt;
    std::string sql;
    char *zErrMsg = 0;
    std::string op;
    if(fileRequest.reqOperator == Less)
        op = "<";
    else if(fileRequest.reqOperator == LessEqual)
        op = "<=";
    else if(fileRequest.reqOperator == Greater)
        op = ">";
    else if(fileRequest.reqOperator == GreaterEqual)
        op = ">=";
    else if(fileRequest.reqOperator == Equal)
        op = "==";

    if(fileRequest.reqOperator == Nothing)
        sql = "select count(*) from FILES where NAME = '"+std::string(fileRequest.fileName)+"';";
    else
        sql = "select count(*) from FILES where NAME = '"+std::string(fileRequest.fileName)+"' AND SIZE "+op +" "+
                std::to_string(fileRequest.n)+";";

        if(sqlite3_open("node.db", &db) != SQLITE_OK) {
        printf("ERROR: can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        // return Failure;
    }

    if(sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL) != SQLITE_OK) {
        printf("ERROR: while compiling sql: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        sqlite3_finalize(stmt);
        //  return Failure;
    }
    int ret_code = 0;
    ret_code = sqlite3_step(stmt);

    int rowcount = sqlite3_column_int(stmt, 0);
    if(rowcount > 0) {
        sqlite3_close(db);

        return Success;
    }
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return Failure;
}
void Node::sendFileToRequestingSuperNode(FileRequest fileRequest) {
    int sd;
    Result result = Network::makeRequest(fileRequest.ipOfTheSuperNodeRequesting, htons(atoi("2908")), SendFileInfoToRequestingSuperNode, sd);
    if (result == Success) {
        Network::send(sd, fileRequest);
    }
    close(sd);
}
in_addr_t Node::getIp()
{
    if(ip == 0) {
        const char *google_dns_server = "8.8.8.8";
        int dns_port = 53;
        struct sockaddr_in serv;
        int sock = socket(AF_INET, SOCK_DGRAM, 0);
        if (sock < 0) {
            perror("Socket error");
        }
        memset(&serv, 0, sizeof(serv));
        serv.sin_family = AF_INET;
        serv.sin_addr.s_addr = inet_addr(google_dns_server);
        serv.sin_port = htons(dns_port);
        int err = connect(sock, (const struct sockaddr *) &serv, sizeof(serv));
        struct sockaddr_in name;
        socklen_t namelen = sizeof(name);
        err = getsockname(sock, (struct sockaddr *) &name, &namelen);
        close(sock);
        return name.sin_addr.s_addr;
    }
    return ip;
}
void Node::searchFile(std::string fileName, Operators reqOperator, int n) {

    int sd;
    FileRequest fileRequest;
    strcpy(fileRequest.fileName, fileName.c_str());
    fileRequest.ipOfTheNodeRequesting = getIp();
    fileRequest.ipOfTheNodeWithFile = 0;
    fileRequest.reqOperator = reqOperator;
    fileRequest.n = n;
    Result result = Network::makeRequest(this->ipSuperNode, htons(atoi("2908")), RequestFileFromConnectedNode, sd);
    if (result == Success) {
        Network::send(sd, fileRequest);
        Result result;
        if (read(sd, &result, sizeof(Result)) <= 0) {
            perror("Eroare la read().\n");
        }
        if(result == Success)
        {
            printf("The requested file has been found!\n");
       //     this->initiateFileTransferRequest(fileRequest);
            printf("after  this->initiateFileTransferRequest(\n");
        }
        else if(result == SearchInOtherSuperNodes)
        {
            printf("Please wait the file is being searched in other super nodes!\n");
        }
        else if(result == Failure)
        {
            printf("The requested file has not been found!\n");
        }
    }
}
void Node::initiateFileTransferRequest(FileRequest fileRequest) {
    int sd;
    Result result = Network::makeRequest(fileRequest.ipOfTheNodeWithFile, htons(atoi("2908")), InitiateFileTransfer, sd);
    if(result == Success)
    {
        int fileSize;
        Network::send(sd, fileRequest);
        if (read(sd, &fileSize, sizeof(int)) <= 0) {
            perror("Eroare la read().\n");
        }
        char buffer[BUFSIZ];
        ssize_t readSoFar = 0;
        ssize_t currentRead = 0;
      //  FILE* fd = fopen(fileRequest.fileName, "w");
        printf("reading...\n");
        printf("Expected size: %d\n",fileSize);
        ssize_t currentWrote;
        int fd = open(fileRequest.fileName, O_WRONLY | O_CREAT | O_TRUNC,0777);

        fd = open(fileRequest.fileName, O_WRONLY  | O_APPEND);

        while(readSoFar < fileSize && (currentRead = read(sd, &buffer, BUFSIZ)) > 0)
        {
           // buffer[currentRead] = '\0';
         //   printf("current buffer1: %s\n",buffer);
            //fprintf(f, "%s", buffer);

            if((currentWrote = write(fd,buffer,currentRead)) != currentRead)
                printf("ERROR\n");
            printf("currentRead = %d\n", currentRead);

            printf("currentWrote = %d\n",currentWrote);
            readSoFar += currentRead;

        }
        close(fd);
        if(fileSize == readSoFar)
            printf("File received succesfully!\n");
       else
            printf("Error when receiving file!\n");
        close(sd);
    }
}
void Node::initiateFileTransferSend(int sd, FileRequest fileRequest) {
        int fileSize = getFileSize(fileRequest);
        if (write(sd, &fileSize, sizeof(int)) <= 0) {
            perror("Eroare la write().\n");
        }
        char buffer[BUFSIZ];
        ssize_t currentSent = 0;
       int fd = open(fileRequest.fileName, O_RDONLY);

        while((currentSent = read(fd, &buffer,BUFSIZ)) > 0)
        {
            write(sd, &buffer, currentSent);
        }
    close(fd);
}
void Node::showSharedFiles() {
    Result result = Failure;
    sqlite3* db;
    sqlite3_stmt* stmt;
    std::string sql;
    char *zErrMsg = 0;

    sql = "select * from FILES;";

    if(sqlite3_open("node.db", &db) != SQLITE_OK) {
        printf("ERROR: can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
       // return Failure;
    }

    if(sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL) != SQLITE_OK) {
        printf("ERROR: while compiling sql: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        sqlite3_finalize(stmt);
      //  return Failure;
    }
    int ret_code = 0;
    ret_code = sqlite3_exec(db, sql.c_str(), callback, 0, &zErrMsg);
    if (ret_code != SQLITE_OK) {
        printf("Error when selecting!\n");
    }
  //  sqlite3_finalize(stmt);
    sqlite3_close(db);
}

