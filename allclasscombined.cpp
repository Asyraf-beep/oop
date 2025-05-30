#include <iostream>
#include <vector>
#include <string>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <sstream>
#include <algorithm>

const int MAX_ROWS = 80;
const int MAX_COLS = 50;

/*Starting from here is all of the class definition
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
*/

class Robot;

class Logger {
    std::ofstream logFile;

public:
    Logger(const std::string& filename) ;
    ~Logger() ;

    void log(const std::string& message) ;
};

class Battlefield {
    int rows, cols, steps;
    std::vector<std::vector<std::string>> grid;
    std::vector<Robot*> robots;                     //pointer
    std::vector<Robot*> graveyard ;                 //queue
    Logger* logger;

public:
    Battlefield(int r, int c) : rows(r), cols(c), grid(r, std::vector<std::string>(c, "+___")) {
        logger = new Logger("log.txt") ;
    }
    ~Battlefield();

    int getRows() ;
    int getCols() ;
    int getSteps() ;
    std::vector<Robot*>& getRobots() ;
    Logger* getLogger() ;

    void setRows(int row) ;
    void setCols(int col) ;
    void setSteps(int step) ;

    void loadFromFile(const std::string& filename);
    void runSimulation();
    void display();
    bool isInside(int x, int y);
    bool isOccupied(int x, int y);
    void createRobot(Robot* robot);
    void enterGraveyard(Robot* robot) ;
    void reviveOne() ;
};

class Robot {
protected:
    std::string name, type;
    int posX, posY;
    int lives = 1;
    int revivals = 3 ;
    Battlefield* battlefield ;

public:
    Robot(std::string t, std::string n, int x, int y , Battlefield* bf) :
        type(t), name(n), posX(x), posY(y) , battlefield(bf) {}

    virtual void takeTurn() = 0;
    virtual bool isAlive() const { return lives > 0; }
    std::string getName() const { return name; }
    int getX() const { return posX; }
    int getY() const { return posY; }
    int getRevival() const { return revivals ; }
    void setPosition(int x, int y) { posX = x; posY = y; }
    bool canRevive() { return revivals > 0 ; }
    void usingRevival() { revivals-- ;}
    virtual void takeDamage() {
        lives--;
        battlefield->getLogger()->log(name + " is taking damage!\n") ;
    }
    void kill() { lives = 0 ; }
    virtual void reset() { lives = 1 ; }
    virtual ~Robot() = default;
};

class MovingRobot : virtual public Robot {
public:
    using Robot::Robot;
    virtual void move(int dx, int dy) ;
    virtual ~MovingRobot() = default;
};

class ShootingRobot : virtual public Robot {
protected:
    int shells = 10 ;
public:
    using Robot::Robot;
    virtual void fire(int dx, int dy) ;
    int getShells() ;
    void setShells(int shell) ;
    virtual ~ShootingRobot() = default;
};

class SeeingRobot : virtual public Robot {
public:
    using Robot::Robot;
    virtual void look(int dx, int dy) ;
    virtual ~SeeingRobot() = default;
};

class ThinkingRobot : virtual public Robot {
public:
    using Robot::Robot;
    virtual void think() ;
    virtual ~ThinkingRobot() = default;
};

class GenericRobot : public MovingRobot, public ShootingRobot, public SeeingRobot, public ThinkingRobot {
protected:

public:
    GenericRobot(std::string& type, std::string& name, int x, int y, Battlefield* bf)
        : Robot(type, name, x, y, bf),
          MovingRobot(), ShootingRobot(), SeeingRobot(), ThinkingRobot() {}


    void takeTurn() override ;

    void reset() override ;
};

class HideBot : public GenericRobot {                //override takeDamage()
protected:
    int remainingHides = 3;
public:
    HideBot(std::string& type, std::string& name, int x, int y, Battlefield* bf)
        : Robot(type, name, x, y, bf), GenericRobot(type, name, x, y, bf) {}

    bool canHide() ;
    void takeDamage() override ;
};

class JumpBot : public GenericRobot {       //override move()
private:
    int remainingJumps = 3;

public:
    JumpBot(std::string type, std::string name, int x, int y, Battlefield* bf)
        : Robot(type, name, x, y, bf), GenericRobot(type, name, x, y, bf) {}

    void move(int dx, int dy) override ;

    bool canJump() ;
};

class JuggernautBot : public GenericRobot {
private:
    std::string directions[4] = {"up" , "down" , "left" , "right"} ;

public:
    JuggernautBot(std::string type, std::string name, int x, int y, Battlefield* bf)
        : Robot(type, name, x, y, bf), GenericRobot(type, name, x, y, bf) {}

    void move(int dx, int dy) override ;
};

/*Starting from here is all of the full functions
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
*/
void MovingRobot::move(int dx, int dy) {
    int newX = posX + dx;
    int newY = posY + dy;

    battlefield->getLogger()->log(name + " want to move to (" + std::to_string(newX) + "," + std::to_string(newY) + ")\n") ;

    if(battlefield->isInside(newX, newY) && !battlefield->isOccupied(newX, newY)) {
        posX = newX;
        posY = newY;
        battlefield->getLogger()->log(name + " moves to (" + std::to_string(posX) + ", " + std::to_string(posY) + ")\n") ;
    }
    else {
        battlefield->getLogger()->log("Cannot move to (" + std::to_string(newX) + "," + std::to_string(newY) + ") : Invalid Position\n") ;
    }
}

void ShootingRobot::fire(int dx, int dy) {
    if ((dx == 0 && dy == 0))
        return;

    if(shells > 0) {
        shells--;

        int targetX = posX + dx;
        int targetY = posY + dy;

        if (!battlefield->isInside(targetX, targetY)) {                     //only shoot inside the battlefield area
            battlefield->getLogger()->log(name + " tried to fire outside the battlefield.\n") ;
            return;
        }

        battlefield->getLogger()->log(name + " fires at (" + std::to_string(targetX) + ", " + std::to_string(targetY) + ")\n") ;

        for (Robot* other : battlefield->getRobots()) {                            //check if there is robot there
            if (other->isAlive() && other->getX() == targetX && other->getY() == targetY) {

                battlefield->getLogger()->log(name + " hits " + other->getName() + "!\n") ;
                other->takeDamage();

                break; // Only one robot can be at a position
            }
        }
    }
    else {
        battlefield->getLogger()->log(name + " is out of shells and self-destructs!\n") ;
        kill();
    }
}

int ShootingRobot::getShells() {
    return shells ;
}

void ShootingRobot::setShells(int shell) {
    shells = shell ;
}

void SeeingRobot::look(int dx, int dy) {
    int targetX = posX + dx ;
    int targetY = posY + dy ;
    std::vector<std::pair<int,int>> lookAreas = {
        {targetX - 1 , targetY - 1}, {targetX , targetY - 1}, {targetX + 1 , targetY - 1},
        {targetX - 1 , targetY}    , {targetX , targetY}    , {targetX + 1 , targetY}    ,
        {targetX - 1 , targetY + 1}, {targetX , targetY + 1}, {targetX + 1 , targetY + 1}
    };
    battlefield->getLogger()->log(name + " is looking at (" + std::to_string(targetX) + ", " + std::to_string(targetY) + ")\n") ;

    std::vector<std::pair<int,int>> foundRobot ;

    for(Robot* other : battlefield->getRobots()) {
        for(std::pair<int,int>& lookArea : lookAreas) {
            if(other != this && other->isAlive() && other->getX() == lookArea.first && other->getY() == lookArea.second) {
                battlefield->getLogger()->log(name + " found " + other->getName() + " at (" + std::to_string(other->getX()) + "," + std::to_string(other->getY()) + ")\n") ;
                foundRobot.push_back({other->getX() , other->getY()}) ;
            }
        }
    }
}

void ThinkingRobot::think() {
    battlefield->getLogger()->log(name + " is thinking about its next move.\n") ;
}

void GenericRobot::takeTurn() {

    think() ;

    int dx = rand() % 3 - 1;
    int dy = rand() % 3 - 1;

    look(dx, dy);

    dx = rand() % 3 - 1;
    dy = rand() % 3 - 1;

    fire(dx, dy);

    dx = rand() % 3 - 1;
    dy = rand() % 3 - 1;

    move(dx, dy);
}

void GenericRobot::reset() {
    lives = 1 ;
    shells = 10 ;
}

bool HideBot::canHide() {
    return remainingHides > 0 ;
}

void HideBot::takeDamage() {
    if(canHide()) {
        remainingHides--;
        battlefield->getLogger()->log(name + " is hiding and avoid the hit (invulnerable). Hides left: " + std::to_string(remainingHides) + "\n") ;
        return ;
    }
    else {
        lives-- ;
        battlefield->getLogger()->log(name + " tried to hide but has no hides left! TAKING DAMAGE!\n") ;
    }
}

void JumpBot::move(int dx, int dy) {                 //changed from jump() to just overriding move()
    if(canJump()) {
        int newX = rand() % battlefield->getCols();       //random position inside the boundaries
        int newY = rand() % battlefield->getRows();

        if(battlefield->isInside(newX, newY) && !battlefield->isOccupied(newX, newY)) {
            setPosition(newX, newY);
            remainingJumps--;
            battlefield->getLogger()->log(name + " jumped to (" + std::to_string(newX) + ", "
                                          + std::to_string(newY) + "). Jumps left: " + std::to_string(remainingJumps) + "\n") ;
        }
        else {
            battlefield->getLogger()->log(name + " tried to jump to (" + std::to_string(newX) + "," + std::to_string(newY)
                                          + "). Invalid Position. No Jumps consumed.\n") ;
        }
    }
    else {
        battlefield->getLogger()->log(name + " tried to jump but has no jumps left! Proceed with normal movement logic\n") ;

        int newX = posX + dx;
        int newY = posY + dy;
        battlefield->getLogger()->log(name + " want to move to (" + std::to_string(newX) + "," + std::to_string(newY) + ")\n") ;

        if(battlefield->isInside(newX, newY) && !battlefield->isOccupied(newX, newY)) {
            setPosition(newX , newY) ;
            battlefield->getLogger()->log(name + " moves to (" + std::to_string(posX) + ", " + std::to_string(posY) + ")\n") ;
        }
        else {
            battlefield->getLogger()->log("Cannot move to (" + std::to_string(newX) + "," + std::to_string(newY) + ") : Invalid Position\n") ;
        }
    }
}

bool JumpBot::canJump() {
    return remainingJumps > 0 ;
}

void JuggernautBot::move(int dx, int dy) {
    int idxDirection = rand() % 4 ;               //randomize direction
    int oldY = posY ;
    int oldX = posX ;                             //for display purposes

    if(directions[idxDirection] == "up") {
        if(posY == 0) {
            battlefield->getLogger()->log("At the edge, Cannot move up. Skipping Move.\n") ;
            return ;
        }
        dy = rand() % posY ;                //valid dy to move
        setPosition(posX , posY - dy) ;           //x remain constant, y moving

        battlefield->getLogger()->log(name + " is charging through the line from (" + std::to_string(oldX) + "," + std::to_string(oldY) + ") towards ("
                                      + std::to_string(posX) + "," + std::to_string(posY) + "). Dealing damage to all robot along the path\n") ;

        for(Robot* other : battlefield->getRobots()) {  //dealing damage along passed line
            for(int i = 0 ; i < dy ; i++) {
                if(other != this && other->isAlive() && other->getX() == posX && other->getY() == (posY - i)) {
                    other->takeDamage() ;
                }
            }
        }
    }
    else if(directions[idxDirection] == "down") {
        if(posY == battlefield->getRows()) {
            battlefield->getLogger()->log("At the edge, Cannot move up. Skipping Move.\n") ;
            return ;
        }
        dy = rand() % (battlefield->getRows() - posY) ;
        setPosition(posX , posY + dy) ;

        battlefield->getLogger()->log(name + " is charging through the line from (" + std::to_string(oldX) + "," + std::to_string(oldY) + ") towards ("
                                      + std::to_string(posX) + "," + std::to_string(posY) + "). Dealing damage to all robot along the path\n") ;

        for(Robot* other : battlefield->getRobots()) {
            for(int i = 0 ; i < dy ; i++) {
                if(other != this && other->isAlive() && other->getX() == posX && other->getY() == (posY + i)) {
                    other->takeDamage() ;
                }
            }
        }
    }
    else if(directions[idxDirection] == "left") {
        if(posX == 0) {
            battlefield->getLogger()->log("At the edge, Cannot move up. Skipping Move.\n") ;
            return ;
        }
        dx = rand() % (posX - 0) ;
        setPosition(posX - dx , posY) ;           //y remain constact, x moving

        battlefield->getLogger()->log(name + " is charging through the line from (" + std::to_string(oldX) + "," + std::to_string(oldY) + ") towards ("
                                      + std::to_string(posX) + "," + std::to_string(posY) + "). Dealing damage to all robot along the path\n") ;

        for(Robot* other : battlefield->getRobots()) {
            for(int i = 0 ; i < dx ; i++) {
                if(other != this && other->isAlive() && other->getX() == (posX - i) && other->getY() == posY) {
                    other->takeDamage() ;
                }
            }
        }
    }
    else if(directions[idxDirection] == "right") {
        if(posX == battlefield->getCols()) {
            battlefield->getLogger()->log("At the edge, Cannot move up. Skipping Move.\n") ;
            return ;
        }
        dx = rand() % (battlefield->getCols() - posX) ;
        setPosition(posX + dx , posY) ;

        battlefield->getLogger()->log(name + " is charging through the line from (" + std::to_string(oldX) + "," + std::to_string(oldY) + ") towards ("
                                      + std::to_string(posX) + "," + std::to_string(posY) + "). Dealing damage to all robot along the path\n") ;

        for(Robot* other : battlefield->getRobots()) {
            for(int i = 0 ; i < dx ; i++) {
                if(other != this && other->isAlive() && other->getX() == (posX + i) && other->getY() == posY) {
                    other->takeDamage() ;
                }
            }
        }
    }
}

Logger::Logger(const std::string& filename) {
    logFile.open(filename, std::ios::out);
}

Logger::~Logger() {
    if (logFile.is_open())
        logFile.close();
}

void Logger::log(const std::string& message) {
    std::cout << message ;
    if (logFile.is_open()) {
        logFile << message ;
    }
}

void Battlefield::loadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    std::string line;
    while (getline(file, line)) {
        if (line.find("M by N") != std::string::npos) {
            std::istringstream iss(line);
            std::string dummy;
            int newCols, newRows;

            //M by N : 40 50
            iss >> dummy >> dummy >> dummy >> dummy >> newRows >> newCols;
            setCols(newCols) ;
            setRows(newRows) ;
            grid = std::vector<std::vector<std::string>>(rows, std::vector<std::string>(cols, "+___"));

        } else if (line.find("steps:") != std::string::npos) {
            std::istringstream iss(line);
            std::string dummy;
            iss >> dummy >> steps;
        } else if (line.find("GenericRobot") != std::string::npos) {
            std::istringstream iss(line);
            std::string type, name;
            std::string sx, sy;
            iss >> type >> name >> sx >> sy;

            if(name.size() < 3)
                name = name + "_" ;



            int x = sx == "random" ? (cols > 0 ? rand() % cols : 0) : std::stoi(sx);
            int y = sy == "random" ? (rows > 0 ? rand() % rows : 0) : std::stoi(sy);

            while(true) {
                if(isInside(x, y) && !isOccupied(x, y)) {
                    Robot* robot = new GenericRobot(type, name, x, y, this);
                    createRobot(robot) ;

                    grid[y][x] = name.substr(0,3);
                    getLogger()->log("Loaded robot " + name + " at (" + std::to_string(x) + ", " + std::to_string(y) + ")\n");
                    break ;
                }
                else {
                    getLogger()->log("Invalid Position. Randomizing new position\n") ;
                }

                x = rand() % cols ;
                y = rand() % rows ;
            }
        }
    }
    getLogger()->log("Finished loading file. Battlefield size: " + std::to_string(cols) + "x" + std::to_string(rows)
                                  + ", Steps: " + std::to_string(steps) + ", Robots: " + std::to_string(robots.size()) + "\n") ;
    display() ;
}

void Battlefield::runSimulation() {

    for (int step = 0; step < steps && robots.size() > 1; ++step) {
        getLogger()->log("\nStep: " + std::to_string(step + 1) + "\n");

        reviveOne() ;

        for (Robot* robot : robots) {
            if (robot->isAlive()) {
                robot->takeTurn();
            }
        }
        for (Robot* robot : robots) {         //find ded robot
            if(!robot->isAlive()) {
                getLogger()->log(robot->getName() + " is ded\n") ;
                enterGraveyard(robot) ;
            }
        }

        display();

        getLogger()->log("Graveyard : ") ;                    //display graveyard list
        for(Robot* robot : graveyard) {
            getLogger()->log("[" + robot->getName() + "] ") ;
        }

        getLogger()->log("\n") ;

        int robotCounter = 0 ;
        for(Robot* robot : robots) {
            if(robot->isAlive())
                robotCounter++ ;
        }

        if(robotCounter == 1 && graveyard.empty()) {
            getLogger()->log("Only 1 robot left\n") ;
            break;
        }
    }
}

void Battlefield::display() {
    grid = std::vector<std::vector<std::string>>(rows, std::vector<std::string>(cols, "+___"));
    for (auto& robot : robots) {
        if (robot->isAlive())
            grid[robot->getY()][robot->getX()] = "+" + robot->getName().substr(0, 3);
    }

    std::cout << "+___" ;
    for (int x = 0; x < cols; ++x) {
        if (x < 10)
            std::cout << "+_" << x << "_" ;
        else
            std::cout << "+_" << x ;
    }
    std::cout << "\n";

    for (int y = 0; y < rows; ++y) {
        if (y < 10)
            std::cout << "+_" << y << "_" ;
        else
            std::cout << "+_" << y ;

        for (int x = 0; x < cols; ++x) {
            std::cout << grid[y][x];
        }
        std::cout << "\n";
    }
}

bool Battlefield::isInside(int x, int y) {
    return x >= 0 && y >= 0 && x < cols && y < rows;
}

bool Battlefield::isOccupied(int x, int y) {

    for(Robot* robot : robots) {
        if(robot->isAlive() && robot->getX() == x && robot->getY() == y)
            return true ;

    }

    return false ;
}

void Battlefield::createRobot(Robot* robot) {
    robots.push_back(robot);
}

int Battlefield::getRows() { return rows ; }
int Battlefield::getCols() { return cols ; }
int Battlefield::getSteps() { return steps ; }
std::vector<Robot*>& Battlefield::getRobots() { return robots; }

void Battlefield::setRows(int row) { rows = row ; }
void Battlefield::setCols(int col) { cols = col ; }
void Battlefield::setSteps(int step) { steps = step ; }

void Battlefield::enterGraveyard(Robot* robot) {
    if (std::find(graveyard.begin(), graveyard.end(), robot) == graveyard.end()) {   //check if the robot is already waiting in the queue
        graveyard.push_back(robot) ;
    }

}

void Battlefield::reviveOne() {
    if (!graveyard.empty()) {
        Robot* deadRobot = graveyard.front();

        if(deadRobot->canRevive()) {
            while (true) {                 //loop eternally until we can get unoccupied space
                int newX = rand() % cols;
                int newY = rand() % rows;

                if (!isOccupied(newX, newY)) {
                    deadRobot->setPosition(newX, newY);
                    deadRobot->usingRevival() ;
                    deadRobot->reset(); // Reset lives and shells

                    graveyard.erase(graveyard.begin());  //kick out of the queue
                    getLogger()->log(deadRobot->getName() + " has been revived at (" + std::to_string(newX) + "," + std::to_string(newY) + ")"
                                     + ". Remaining revivals : " + std::to_string(deadRobot->getRevival()) + "\n") ;
                    return ;
                }
            }
        }
        else {
            getLogger()->log("Attempting to revive " + deadRobot->getName() + " but no revives left. let him ascend.\n") ;
            graveyard.erase(graveyard.begin());      //if cannot revive just kick out of the queue
            robots.erase(std::remove(robots.begin(), robots.end(), deadRobot), robots.end());     //destroy his soul
            delete deadRobot ;                       //dont want dangling pointer
        }
    }
}

Logger* Battlefield::getLogger() {
    return logger;
}

Battlefield::~Battlefield() {
    for (Robot* robot : robots) {
        delete robot;
    }

    delete logger ;
}

/* Starting from here in main()
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
*/

int main() {
    srand(static_cast<unsigned>(time(nullptr)));
    Battlefield battlefield(MAX_ROWS, MAX_COLS);
    battlefield.loadFromFile("input.txt");
    battlefield.runSimulation();
    return 0;
}
