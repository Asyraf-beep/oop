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
    void upgrade(Robot* robot) ;
};

class Robot {
protected:
    std::string type, name;
    int posX, posY;
    int lives = 1;
    int revivals = 3 ;
    bool upgradeFirst = false ;
    bool upgradeSecond = false ;
    bool upgradeThird = false ;
    int upgradePoints = 0 ;
    Battlefield* battlefield ;

public:
    Robot(const std::string& t,const std::string& n, int x, int y , Battlefield* bf) :
        type(t), name(n), posX(x), posY(y) , battlefield(bf) {}

    virtual void takeTurn() = 0;
    virtual bool isAlive() const { return lives > 0; }
    bool canRevive() { return revivals > 0 ; }

    std::string getType() const { return type; }
    std::string getName() const { return name; }
    int getX() const { return posX; }
    int getY() const { return posY; }
    int getRevivals() const { return revivals ; }
    bool getUpgradeFirst() { return upgradeFirst ; }
    bool getUpgradeSecond() { return upgradeSecond ; }
    bool getUpgradeThird() { return upgradeThird ; }
    int getUpgradePoints() { return upgradePoints ; }

    void setType(const std::string& type) { this->type = type ; }
    void setName(const std::string& name) { this->name = name ; }
    void setRevivals(int revival) { revivals = revival ; }
    void setPosition(int x, int y) { posX = x; posY = y; }
    void setLives(int live) { lives = live ; }
    void setUpgradeFirst(bool state) { upgradeFirst = state ; }
    void setUpgradeSecond(bool state) { upgradeSecond = state ; }
    void setUpgradeThird(bool state) { upgradeThird = state ; }
    void setUpgradePoints(int upgradePoint) { upgradePoints = upgradePoint ; }

    void subRevivals() { revivals-- ; }
    void addRevivals() { revivals++ ; }
    void addLives() { lives++ ; }
    void subLives() { lives-- ; }
    void addUpgradePoints() { upgradePoints++ ; }
    void subUpgradePoints() { upgradePoints-- ; }
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
private:
    int shells = 10 ;
public:
    using Robot::Robot;
    virtual void fire(int dx, int dy) ;
    int getShells() ;
    void setShells(int shell) ;
    void addShells() { shells++ ; }
    void subShells() { shells-- ; }
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

class GenericRobot : virtual public MovingRobot, virtual public ShootingRobot, virtual public SeeingRobot, virtual public ThinkingRobot {
public:
    GenericRobot(const std::string& type, const std::string& name, int x, int y, Battlefield* bf)
        : Robot(type, name, x, y, bf),
          MovingRobot(), ShootingRobot(), SeeingRobot(), ThinkingRobot() {}

    void takeTurn() override ;
    void reset() override ;

};

class HideBot : virtual public GenericRobot {                //override takeDamage()
private:
    int remainingHides = 3;
public:
    HideBot(const std::string& type, const std::string& name, int x, int y, Battlefield* bf)
        : Robot(type, name, x, y, bf), GenericRobot(type, name, x, y, bf) {}

    bool canHide() ;
    void takeDamage() override ;
};

class JumpBot : virtual public GenericRobot {       //override move()
private:
    int remainingJumps = 3;

public:
    JumpBot(const std::string& type, const std::string& name, int x, int y, Battlefield* bf)
        : Robot(type, name, x, y, bf), GenericRobot(type, name, x, y, bf) {}

    void move(int dx, int dy) override ;

    bool canJump() ;
};

class JuggernautBot : virtual public GenericRobot {
private:
    std::string directions[4] = {"up" , "down" , "left" , "right"} ;

public:
    JuggernautBot(const std::string& type, const std::string& name, int x, int y, Battlefield* bf)
        : Robot(type, name, x, y, bf), GenericRobot(type, name, x, y, bf) {}

    void move(int dx, int dy) override ;
};

class TrueDamageBot : virtual public GenericRobot {
public:
    TrueDamageBot(const std::string& type, const std::string& name, int x, int y, Battlefield* bf)
        : Robot(type, name, x, y, bf), GenericRobot(type, name, x, y, bf) {
        setType("TrueDamageBot") ;
    }

    void fire(int dx, int dy) override ;
};

class LifestealBot : virtual public GenericRobot {
public:
    LifestealBot(const std::string& type, const std::string& name, int x, int y, Battlefield* bf)
        : Robot(type, name, x, y, bf), GenericRobot(type, name, x, y, bf) {
        setType("LifestealBot") ;
    }

    void fire(int dx, int dy) override ;
};

class LongshotBot : virtual public GenericRobot {
public:
    LongshotBot(const std::string& type, const std::string& name, int x, int y, Battlefield* bf)
        : Robot(type, name, x, y, bf), GenericRobot(type, name, x, y, bf) {
        setType("LongshotBot") ;
    }

    void fire(int dx, int dy) override ;
};

class ThirtyshotBot : virtual public GenericRobot {
public:
    ThirtyshotBot(const std::string& type, const std::string& name, int x, int y, Battlefield* bf)
        : Robot(type, name, x, y, bf), GenericRobot(type, name, x, y, bf) {
        setShells(30);         // replace current shell count with 30
    }
};

class SemiautoBot : virtual public GenericRobot {
public:
    SemiautoBot(const std::string& type, const std::string& name, int x, int y, Battlefield* bf)
        : Robot(type, name, x, y, bf), GenericRobot(type, name, x, y, bf) {
        setType("SemiautoBot") ;
    }

    void fire(int dx, int dy) override ;
};

class ScoutBot : virtual public GenericRobot {
public:
    ScoutBot(const std::string& type, const std::string& name, int x, int y, Battlefield* bf)
        : Robot(type, name, x, y, bf), GenericRobot(type, name, x, y, bf) {
        setType("ScoutBot") ;
    }

    void look(int dx, int dy) override ;
};

class TrackerBot : virtual public GenericRobot {
public:
    TrackerBot(const std::string& type, const std::string& name, int x, int y, Battlefield* bf)
        : Robot(type, name, x, y, bf), GenericRobot(type, name, x, y, bf) {
        setType("TrackerBot") ;
    }

    //void look(int dx, int dy) override ;
};

class HideLongshotBot : virtual public HideBot, virtual public LongshotBot {
public:
    HideLongshotBot(const std::string& type, const std::string& name, int x, int y, Battlefield* bf)
        : Robot(type, name, x, y, bf), GenericRobot(type, name, x, y, bf), HideBot(type, name, x, y, bf), LongshotBot(type, name, x, y, bf) {
        setType("HideLongshotBot") ;
    }

    //no need to override anything because there's no ambiguity, because no overlapping functions between seeing shooting moving.
};

class HideSemiautoBot : virtual public HideBot, virtual public SemiautoBot {
public:
    HideSemiautoBot(const std::string& type, const std::string& name, int x, int y, Battlefield* bf)
        : Robot(type, name, x, y, bf), GenericRobot(type, name, x, y, bf), HideBot(type, name, x, y, bf), SemiautoBot(type, name, x, y, bf) {
        setType("HideSemiautoBot") ;
    }

    //no need to override anything because there's no ambiguity, because no overlapping functions between seeing shooting moving.
};

class HideThirtyshotBot : virtual public HideBot, virtual public ThirtyshotBot {
public:
    HideThirtyshotBot(const std::string& type, const std::string& name, int x, int y, Battlefield* bf)
        : Robot(type, name, x, y, bf), GenericRobot(type, name, x, y, bf), HideBot(type, name, x, y, bf), ThirtyshotBot(type, name, x, y, bf) {
        setType("HideThirtyshotBot") ;
    }

    //no need to override anything because there's no ambiguity, because no overlapping functions between seeing shooting moving.
};

class HideTrueDamageBot : virtual public HideBot, virtual public TrueDamageBot {
public:
    HideTrueDamageBot(const std::string& type, const std::string& name, int x, int y, Battlefield* bf)
        : Robot(type, name, x, y, bf), GenericRobot(type, name, x, y, bf), HideBot(type, name, x, y, bf), TrueDamageBot(type, name, x, y, bf) {
        setType("HideTrueDamageBot") ;
    }

    //no need to override anything because there's no ambiguity, because no overlapping functions between seeing shooting moving.
};

class HideLifestealBot : virtual public HideBot, virtual public LifestealBot {
public:
    HideLifestealBot(const std::string& type, const std::string& name, int x, int y, Battlefield* bf)
        : Robot(type, name, x, y, bf), GenericRobot(type, name, x, y, bf), HideBot(type, name, x, y, bf), LifestealBot(type, name, x, y, bf) {
        setType("HideLifestealBot") ;
    }

    //no need to override anything because there's no ambiguity, because no overlapping functions between seeing shooting moving.
};

class JumpLongshotBot : virtual public JumpBot, virtual public LongshotBot {
public:
    JumpLongshotBot(const std::string& type, const std::string& name, int x, int y, Battlefield* bf)
        : Robot(type, name, x, y, bf), GenericRobot(type, name, x, y, bf), JumpBot(type, name, x, y, bf), LongshotBot(type, name, x, y, bf) {
        setType("JumpLongshotBot") ;
    }

    //no need to override anything because there's no ambiguity, because no overlapping functions between seeing shooting moving.
};

class JumpSemiautoBot : virtual public JumpBot, virtual public SemiautoBot {
public:
    JumpSemiautoBot(const std::string& type, const std::string& name, int x, int y, Battlefield* bf)
        : Robot(type, name, x, y, bf), GenericRobot(type, name, x, y, bf), JumpBot(type, name, x, y, bf), SemiautoBot(type, name, x, y, bf) {
        setType("JumpSemiautoBot") ;
    }

    //no need to override anything because there's no ambiguity, because no overlapping functions between seeing shooting moving.
};

class JumpThirtyshotBot : virtual public JumpBot, virtual public ThirtyshotBot {
public:
    JumpThirtyshotBot(const std::string& type, const std::string& name, int x, int y, Battlefield* bf)
        : Robot(type, name, x, y, bf), GenericRobot(type, name, x, y, bf), JumpBot(type, name, x, y, bf), ThirtyshotBot(type, name, x, y, bf) {
        setType("JumpThirtyshotBot") ;
    }

    //no need to override anything because there's no ambiguity, because no overlapping functions between seeing shooting moving.
};

class JumpTrueDamageBot : virtual public JumpBot, virtual public TrueDamageBot {
public:
    JumpTrueDamageBot(const std::string& type, const std::string& name, int x, int y, Battlefield* bf)
        : Robot(type, name, x, y, bf), GenericRobot(type, name, x, y, bf), JumpBot(type, name, x, y, bf), TrueDamageBot(type, name, x, y, bf) {
        setType("JumpTrueDamageBot") ;
    }

    //no need to override anything because there's no ambiguity, because no overlapping functions between seeing shooting moving.
};

class JumpLifestealBot : virtual public JumpBot, virtual public LifestealBot {
public:
    JumpLifestealBot(const std::string& type, const std::string& name, int x, int y, Battlefield* bf)
        : Robot(type, name, x, y, bf), GenericRobot(type, name, x, y, bf), JumpBot(type, name, x, y, bf), LifestealBot(type, name, x, y, bf) {
        setType("JumpLifestealBot") ;
    }

    //no need to override anything because there's no ambiguity, because no overlapping functions between seeing shooting moving.
};

class JuggernautLongshotBot : virtual public JuggernautBot, virtual public LongshotBot {
public:
    JuggernautLongshotBot(const std::string& type, const std::string& name, int x, int y, Battlefield* bf)
        : Robot(type, name, x, y, bf), GenericRobot(type, name, x, y, bf), JuggernautBot(type, name, x, y, bf), LongshotBot(type, name, x, y, bf) {
        setType("JuggernautLongshotBot") ;
    }

    //no need to override anything because there's no ambiguity, because no overlapping functions between seeing shooting moving.
};

class JuggernautSemiautoBot : virtual public JuggernautBot, virtual public SemiautoBot {
public:
    JuggernautSemiautoBot(const std::string& type, const std::string& name, int x, int y, Battlefield* bf)
        : Robot(type, name, x, y, bf), GenericRobot(type, name, x, y, bf), JuggernautBot(type, name, x, y, bf), SemiautoBot(type, name, x, y, bf) {
        setType("JuggernautSemiautoBot") ;
    }

    //no need to override anything because there's no ambiguity, because no overlapping functions between seeing shooting moving.
};

class JuggernautThirtyshotBot : virtual public JuggernautBot, virtual public ThirtyshotBot {
public:
    JuggernautThirtyshotBot(const std::string& type, const std::string& name, int x, int y, Battlefield* bf)
        : Robot(type, name, x, y, bf), GenericRobot(type, name, x, y, bf), JuggernautBot(type, name, x, y, bf), ThirtyshotBot(type, name, x, y, bf) {
        setType("JuggernautThirtyshotBot") ;
    }

    //no need to override anything because there's no ambiguity, because no overlapping functions between seeing shooting moving.
};

class JuggernautTrueDamageBot : virtual public JuggernautBot, virtual public TrueDamageBot {
public:
    JuggernautTrueDamageBot(const std::string& type, const std::string& name, int x, int y, Battlefield* bf)
        : Robot(type, name, x, y, bf), GenericRobot(type, name, x, y, bf), JuggernautBot(type, name, x, y, bf), TrueDamageBot(type, name, x, y, bf) {
        setType("JuggernautTrueDamageBot") ;
    }

    //no need to override anything because there's no ambiguity, because no overlapping functions between seeing shooting moving.
};

class JuggernautLifestealBot : virtual public JuggernautBot, virtual public LifestealBot {
public:
    JuggernautLifestealBot(const std::string& type, const std::string& name, int x, int y, Battlefield* bf)
        : Robot(type, name, x, y, bf), GenericRobot(type, name, x, y, bf), JuggernautBot(type, name, x, y, bf), LifestealBot(type, name, x, y, bf) {
        setType("JuggernautLifestealBot") ;
    }

    //no need to override anything because there's no ambiguity, because no overlapping functions between seeing shooting moving.
};

class HideLongshotScoutBot : virtual public HideLongshotBot, virtual public ScoutBot {
public:
    HideLongshotScoutBot(const std::string& type, const std::string& name, int x, int y, Battlefield* bf)
        : Robot(type, name, x, y, bf),
          GenericRobot(type, name, x, y, bf),
          HideBot(type, name, x, y, bf),
          LongshotBot(type, name, x, y, bf),
          HideLongshotBot(type, name, x, y, bf),
          ScoutBot(type, name, x, y, bf) {
        setType("HideLongshotScoutBot") ;
    }

    //no need to override anything because there's no ambiguity, because no overlapping functions between seeing shooting moving.
};

class HideLongshotTrackerBot : virtual public HideLongshotBot, virtual public TrackerBot {
public:
    HideLongshotTrackerBot(const std::string& type, const std::string& name, int x, int y, Battlefield* bf)
        : Robot(type, name, x, y, bf),
          GenericRobot(type, name, x, y, bf),
          HideBot(type, name, x, y, bf),
          LongshotBot(type, name, x, y, bf),
          HideLongshotBot(type, name, x, y, bf),
          TrackerBot(type, name, x, y, bf) {
        setType("HideLongshotTrackerBot") ;
    }

    //no need to override anything because there's no ambiguity, because no overlapping functions between seeing shooting moving.
};

class HideSemiautoScoutBot : virtual public HideSemiautoBot, virtual public ScoutBot {
public:
    HideSemiautoScoutBot(const std::string& type, const std::string& name, int x, int y, Battlefield* bf)
        : Robot(type, name, x, y, bf),
          GenericRobot(type, name, x, y, bf),
          HideBot(type, name, x, y, bf),
          SemiautoBot(type, name, x, y, bf),
          HideSemiautoBot(type, name, x, y, bf),
          ScoutBot(type, name, x, y, bf) {
        setType("HideSemiautoScoutBot") ;
    }

    //no need to override anything because there's no ambiguity, because no overlapping functions between seeing shooting moving.
};

class HideSemiautoTrackerBot : virtual public HideSemiautoBot, virtual public TrackerBot {
public:
    HideSemiautoTrackerBot(const std::string& type, const std::string& name, int x, int y, Battlefield* bf)
        : Robot(type, name, x, y, bf),
          GenericRobot(type, name, x, y, bf),
          HideBot(type, name, x, y, bf),
          SemiautoBot(type, name, x, y, bf),
          HideSemiautoBot(type, name, x, y, bf),
          TrackerBot(type, name, x, y, bf) {
        setType("HideSemiautoTrackerBot") ;
    }

    //no need to override anything because there's no ambiguity, because no overlapping functions between seeing shooting moving.
};

class HideThirtyshotScoutBot : virtual public HideThirtyshotBot, virtual public ScoutBot {
public:
    HideThirtyshotScoutBot(const std::string& type, const std::string& name, int x, int y, Battlefield* bf)
        : Robot(type, name, x, y, bf),
          GenericRobot(type, name, x, y, bf),
          HideBot(type, name, x, y, bf),
          ThirtyshotBot(type, name, x, y, bf),
          HideThirtyshotBot(type, name, x, y, bf),
          ScoutBot(type, name, x, y, bf) {
        setType("HideThirtyshotScoutBot") ;
    }

    //no need to override anything because there's no ambiguity, because no overlapping functions between seeing shooting moving.
};

class HideThirtyshotTrackerBot : virtual public HideThirtyshotBot, virtual public TrackerBot {
public:
    HideThirtyshotTrackerBot(const std::string& type, const std::string& name, int x, int y, Battlefield* bf)
        : Robot(type, name, x, y, bf),
          GenericRobot(type, name, x, y, bf),
          HideBot(type, name, x, y, bf),
          ThirtyshotBot(type, name, x, y, bf),
          HideThirtyshotBot(type, name, x, y, bf),
          TrackerBot(type, name, x, y, bf) {
        setType("HideThirtyshotTrackerBot") ;
    }

    //no need to override anything because there's no ambiguity, because no overlapping functions between seeing shooting moving.
};

class HideTrueDamageScoutBot : virtual public HideTrueDamageBot, virtual public ScoutBot {
public:
    HideTrueDamageScoutBot(const std::string& type, const std::string& name, int x, int y, Battlefield* bf)
        : Robot(type, name, x, y, bf),
          GenericRobot(type, name, x, y, bf),
          HideBot(type, name, x, y, bf),
          TrueDamageBot(type, name, x, y, bf),
          HideTrueDamageBot(type, name, x, y, bf),
          ScoutBot(type, name, x, y, bf) {
        setType("HideTrueDamageScoutBot") ;
    }

    //no need to override anything because there's no ambiguity, because no overlapping functions between seeing shooting moving.
};

class HideTrueDamageTrackerBot : virtual public HideTrueDamageBot, virtual public TrackerBot {
public:
    HideTrueDamageTrackerBot(const std::string& type, const std::string& name, int x, int y, Battlefield* bf)
        : Robot(type, name, x, y, bf),
          GenericRobot(type, name, x, y, bf),
          HideBot(type, name, x, y, bf),
          TrueDamageBot(type, name, x, y, bf),
          HideTrueDamageBot(type, name, x, y, bf),
          TrackerBot(type, name, x, y, bf) {
        setType("HideTrueDamageTrackerBot") ;
    }

    //no need to override anything because there's no ambiguity, because no overlapping functions between seeing shooting moving.
};

class HideLifestealScoutBot : virtual public HideLifestealBot, virtual public ScoutBot {
public:
    HideLifestealScoutBot(const std::string& type, const std::string& name, int x, int y, Battlefield* bf)
        : Robot(type, name, x, y, bf),
          GenericRobot(type, name, x, y, bf),
          HideBot(type, name, x, y, bf),
          LifestealBot(type, name, x, y, bf),
          HideLifestealBot(type, name, x, y, bf),
          ScoutBot(type, name, x, y, bf) {
        setType("HideLifestealScoutBot") ;
    }

    //no need to override anything because there's no ambiguity, because no overlapping functions between seeing shooting moving.
};

class HideLifestealTrackerBot : virtual public HideLifestealBot, virtual public TrackerBot {
public:
    HideLifestealTrackerBot(const std::string& type, const std::string& name, int x, int y, Battlefield* bf)
        : Robot(type, name, x, y, bf),
          GenericRobot(type, name, x, y, bf),
          HideBot(type, name, x, y, bf),
          LifestealBot(type, name, x, y, bf),
          HideLifestealBot(type, name, x, y, bf),
          TrackerBot(type, name, x, y, bf) {
        setType("HideLifestealTrackerBot") ;
    }

    //no need to override anything because there's no ambiguity, because no overlapping functions between seeing shooting moving.
};

class JumpLongshotScoutBot : virtual public JumpLongshotBot, virtual public ScoutBot {
public:
    JumpLongshotScoutBot(const std::string& type, const std::string& name, int x, int y, Battlefield* bf)
        : Robot(type, name, x, y, bf),
          GenericRobot(type, name, x, y, bf),
          JumpBot(type, name, x, y, bf),
          LongshotBot(type, name, x, y, bf),
          JumpLongshotBot(type, name, x, y, bf),
          ScoutBot(type, name, x, y, bf) {
        setType("JumpLongshotScoutBot") ;
    }

    //no need to override anything because there's no ambiguity, because no overlapping functions between seeing shooting moving.
};

class JumpLongshotTrackerBot : virtual public JumpLongshotBot, virtual public TrackerBot {
public:
    JumpLongshotTrackerBot(const std::string& type, const std::string& name, int x, int y, Battlefield* bf)
        : Robot(type, name, x, y, bf),
          GenericRobot(type, name, x, y, bf),
          JumpBot(type, name, x, y, bf),
          LongshotBot(type, name, x, y, bf),
          JumpLongshotBot(type, name, x, y, bf),
          TrackerBot(type, name, x, y, bf) {
        setType("JumpLongshotTrackerBot") ;
    }

    //no need to override anything because there's no ambiguity, because no overlapping functions between seeing shooting moving.
};

class JumpSemiautoScoutBot : virtual public JumpSemiautoBot, virtual public ScoutBot {
public:
    JumpSemiautoScoutBot(const std::string& type, const std::string& name, int x, int y, Battlefield* bf)
        : Robot(type, name, x, y, bf),
          GenericRobot(type, name, x, y, bf),
          JumpBot(type, name, x, y, bf),
          SemiautoBot(type, name, x, y, bf),
          JumpSemiautoBot(type, name, x, y, bf),
          ScoutBot(type, name, x, y, bf) {
        setType("JumpSemiautoScoutBot") ;
    }

    //no need to override anything because there's no ambiguity, because no overlapping functions between seeing shooting moving.
};

class JumpSemiautoTrackerBot : virtual public JumpSemiautoBot, virtual public TrackerBot {
public:
    JumpSemiautoTrackerBot(const std::string& type, const std::string& name, int x, int y, Battlefield* bf)
        : Robot(type, name, x, y, bf),
          GenericRobot(type, name, x, y, bf),
          JumpBot(type, name, x, y, bf),
          SemiautoBot(type, name, x, y, bf),
          JumpSemiautoBot(type, name, x, y, bf),
          TrackerBot(type, name, x, y, bf) {
        setType("JumpSemiautoTrackerBot") ;
    }

    //no need to override anything because there's no ambiguity, because no overlapping functions between seeing shooting moving.
};

class JumpThirtyshotScoutBot : virtual public JumpThirtyshotBot, virtual public ScoutBot {
public:
    JumpThirtyshotScoutBot(const std::string& type, const std::string& name, int x, int y, Battlefield* bf)
        : Robot(type, name, x, y, bf),
          GenericRobot(type, name, x, y, bf),
          JumpBot(type, name, x, y, bf),
          ThirtyshotBot(type, name, x, y, bf),
          JumpThirtyshotBot(type, name, x, y, bf),
          ScoutBot(type, name, x, y, bf) {
        setType("JumpThirtyshotScoutBot") ;
    }

    //no need to override anything because there's no ambiguity, because no overlapping functions between seeing shooting moving.
};

class JumpThirtyshotTrackerBot : virtual public JumpThirtyshotBot, virtual public TrackerBot {
public:
    JumpThirtyshotTrackerBot(const std::string& type, const std::string& name, int x, int y, Battlefield* bf)
        : Robot(type, name, x, y, bf),
          GenericRobot(type, name, x, y, bf),
          JumpBot(type, name, x, y, bf),
          ThirtyshotBot(type, name, x, y, bf),
          JumpThirtyshotBot(type, name, x, y, bf),
          TrackerBot(type, name, x, y, bf) {
        setType("JumpThirtyshotTrackerBot") ;
    }

    //no need to override anything because there's no ambiguity, because no overlapping functions between seeing shooting moving.
};

class JumpTrueDamageScoutBot : virtual public JumpTrueDamageBot, virtual public ScoutBot {
public:
    JumpTrueDamageScoutBot(const std::string& type, const std::string& name, int x, int y, Battlefield* bf)
        : Robot(type, name, x, y, bf),
          GenericRobot(type, name, x, y, bf),
          JumpBot(type, name, x, y, bf),
          TrueDamageBot(type, name, x, y, bf),
          JumpTrueDamageBot(type, name, x, y, bf),
          ScoutBot(type, name, x, y, bf) {
        setType("JumpTrueDamageScoutBot") ;
    }

    //no need to override anything because there's no ambiguity, because no overlapping functions between seeing shooting moving.
};

class JumpTrueDamageTrackerBot : virtual public JumpTrueDamageBot, virtual public TrackerBot {
public:
    JumpTrueDamageTrackerBot(const std::string& type, const std::string& name, int x, int y, Battlefield* bf)
        : Robot(type, name, x, y, bf),
          GenericRobot(type, name, x, y, bf),
          JumpBot(type, name, x, y, bf),
          TrueDamageBot(type, name, x, y, bf),
          JumpTrueDamageBot(type, name, x, y, bf),
          TrackerBot(type, name, x, y, bf) {
        setType("JumpTrueDamageTrackerBot") ;
    }

    //no need to override anything because there's no ambiguity, because no overlapping functions between seeing shooting moving.
};

class JumpLifestealScoutBot : virtual public JumpLifestealBot, virtual public ScoutBot {
public:
    JumpLifestealScoutBot(const std::string& type, const std::string& name, int x, int y, Battlefield* bf)
        : Robot(type, name, x, y, bf),
          GenericRobot(type, name, x, y, bf),
          JumpBot(type, name, x, y, bf),
          LifestealBot(type, name, x, y, bf),
          JumpLifestealBot(type, name, x, y, bf),
          ScoutBot(type, name, x, y, bf) {
        setType("JumpLifestealScoutBot") ;
    }

    //no need to override anything because there's no ambiguity, because no overlapping functions between seeing shooting moving.
};

class JumpLifestealTrackerBot : virtual public JumpLifestealBot, virtual public TrackerBot {
public:
    JumpLifestealTrackerBot(const std::string& type, const std::string& name, int x, int y, Battlefield* bf)
        : Robot(type, name, x, y, bf),
          GenericRobot(type, name, x, y, bf),
          JumpBot(type, name, x, y, bf),
          LifestealBot(type, name, x, y, bf),
          JumpLifestealBot(type, name, x, y, bf),
          TrackerBot(type, name, x, y, bf) {
        setType("JumpLifestealTrackerBot") ;
    }

    //no need to override anything because there's no ambiguity, because no overlapping functions between seeing shooting moving.
};

class JuggernautLongshotScoutBot : virtual public JuggernautLongshotBot, virtual public ScoutBot {
public:
    JuggernautLongshotScoutBot(const std::string& type, const std::string& name, int x, int y, Battlefield* bf)
        : Robot(type, name, x, y, bf),
          GenericRobot(type, name, x, y, bf),
          JuggernautBot(type, name, x, y, bf),
          LongshotBot(type, name, x, y, bf),
          JuggernautLongshotBot(type, name, x, y, bf),
          ScoutBot(type, name, x, y, bf) {
        setType("JuggernautLongshotScoutBot") ;
    }

    //no need to override anything because there's no ambiguity, because no overlapping functions between seeing shooting moving.
};

class JuggernautLongshotTrackerBot : virtual public JuggernautLongshotBot, virtual public TrackerBot {
public:
    JuggernautLongshotTrackerBot(const std::string& type, const std::string& name, int x, int y, Battlefield* bf)
        : Robot(type, name, x, y, bf),
          GenericRobot(type, name, x, y, bf),
          JuggernautBot(type, name, x, y, bf),
          LongshotBot(type, name, x, y, bf),
          JuggernautLongshotBot(type, name, x, y, bf),
          TrackerBot(type, name, x, y, bf) {
        setType("JuggernautLongshotTrackerBot") ;
    }

    //no need to override anything because there's no ambiguity, because no overlapping functions between seeing shooting moving.
};

class JuggernautSemiautoScoutBot : virtual public JuggernautSemiautoBot, virtual public ScoutBot {
public:
    JuggernautSemiautoScoutBot(const std::string& type, const std::string& name, int x, int y, Battlefield* bf)
        : Robot(type, name, x, y, bf),
          GenericRobot(type, name, x, y, bf),
          JuggernautBot(type, name, x, y, bf),
          SemiautoBot(type, name, x, y, bf),
          JuggernautSemiautoBot(type, name, x, y, bf),
          ScoutBot(type, name, x, y, bf) {
        setType("JuggernautSemiautoScoutBot") ;
    }

    //no need to override anything because there's no ambiguity, because no overlapping functions between seeing shooting moving.
};

class JuggernautSemiautoTrackerBot : virtual public JuggernautSemiautoBot, virtual public TrackerBot {
public:
    JuggernautSemiautoTrackerBot(const std::string& type, const std::string& name, int x, int y, Battlefield* bf)
        : Robot(type, name, x, y, bf),
          GenericRobot(type, name, x, y, bf),
          JuggernautBot(type, name, x, y, bf),
          SemiautoBot(type, name, x, y, bf),
          JuggernautSemiautoBot(type, name, x, y, bf),
          TrackerBot(type, name, x, y, bf) {
        setType("JuggernautSemiautoTrackerBot") ;
    }

    //no need to override anything because there's no ambiguity, because no overlapping functions between seeing shooting moving.
};

class JuggernautThirtyshotScoutBot : virtual public JuggernautThirtyshotBot, virtual public ScoutBot {
public:
    JuggernautThirtyshotScoutBot(const std::string& type, const std::string& name, int x, int y, Battlefield* bf)
        : Robot(type, name, x, y, bf),
          GenericRobot(type, name, x, y, bf),
          JuggernautBot(type, name, x, y, bf),
          ThirtyshotBot(type, name, x, y, bf),
          JuggernautThirtyshotBot(type, name, x, y, bf),
          ScoutBot(type, name, x, y, bf) {
        setType("JuggernautThirtyshotScoutBot") ;
    }

    //no need to override anything because there's no ambiguity, because no overlapping functions between seeing shooting moving.
};

class JuggernautThirtyshotTrackerBot : virtual public JuggernautThirtyshotBot, virtual public TrackerBot {
public:
    JuggernautThirtyshotTrackerBot(const std::string& type, const std::string& name, int x, int y, Battlefield* bf)
        : Robot(type, name, x, y, bf),
          GenericRobot(type, name, x, y, bf),
          JuggernautBot(type, name, x, y, bf),
          ThirtyshotBot(type, name, x, y, bf),
          JuggernautThirtyshotBot(type, name, x, y, bf),
          TrackerBot(type, name, x, y, bf) {
        setType("JuggernautThirtyshotTrackerBot") ;
    }

    //no need to override anything because there's no ambiguity, because no overlapping functions between seeing shooting moving.
};

class JuggernautTrueDamageScoutBot : virtual public JuggernautTrueDamageBot, virtual public ScoutBot {
public:
    JuggernautTrueDamageScoutBot(const std::string& type, const std::string& name, int x, int y, Battlefield* bf)
        : Robot(type, name, x, y, bf),
          GenericRobot(type, name, x, y, bf),
          JuggernautBot(type, name, x, y, bf),
          TrueDamageBot(type, name, x, y, bf),
          JuggernautTrueDamageBot(type, name, x, y, bf),
          ScoutBot(type, name, x, y, bf) {
        setType("JuggernautTrueDamageScoutBot") ;
    }

    //no need to override anything because there's no ambiguity, because no overlapping functions between seeing shooting moving.
};

class JuggernautTrueDamageTrackerBot : virtual public JuggernautTrueDamageBot, virtual public TrackerBot {
public:
    JuggernautTrueDamageTrackerBot(const std::string& type, const std::string& name, int x, int y, Battlefield* bf)
        : Robot(type, name, x, y, bf),
          GenericRobot(type, name, x, y, bf),
          JuggernautBot(type, name, x, y, bf),
          TrueDamageBot(type, name, x, y, bf),
          JuggernautTrueDamageBot(type, name, x, y, bf),
          TrackerBot(type, name, x, y, bf) {
        setType("JuggernautTrueDamageTrackerBot") ;
    }

    //no need to override anything because there's no ambiguity, because no overlapping functions between seeing shooting moving.
};

class JuggernautLifestealScoutBot : virtual public JuggernautLifestealBot, virtual public ScoutBot {
public:
    JuggernautLifestealScoutBot(const std::string& type, const std::string& name, int x, int y, Battlefield* bf)
        : Robot(type, name, x, y, bf),
          GenericRobot(type, name, x, y, bf),
          JuggernautBot(type, name, x, y, bf),
          LifestealBot(type, name, x, y, bf),
          JuggernautLifestealBot(type, name, x, y, bf),
          ScoutBot(type, name, x, y, bf) {
        setType("JuggernautLifestealScoutBot") ;
    }

    //no need to override anything because there's no ambiguity, because no overlapping functions between seeing shooting moving.
};

class JuggernautLifestealTrackerBot : virtual public JuggernautLifestealBot, virtual public TrackerBot {
public:
    JuggernautLifestealTrackerBot(const std::string& type, const std::string& name, int x, int y, Battlefield* bf)
        : Robot(type, name, x, y, bf),
          GenericRobot(type, name, x, y, bf),
          JuggernautBot(type, name, x, y, bf),
          LifestealBot(type, name, x, y, bf),
          JuggernautLifestealBot(type, name, x, y, bf),
          TrackerBot(type, name, x, y, bf) {
        setType("JuggernautLifestealTrackerBot") ;
    }

    //no need to override anything because there's no ambiguity, because no overlapping functions between seeing shooting moving.
};

/*Starting from here is all of the full functions
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
*/
void MovingRobot::move(int dx, int dy) {
    if(dx == 0 && dy == 0)
        return ;

    int newX = getX() + dx;
    int newY = getY() + dy;

    battlefield->getLogger()->log(getName() + " want to move to (" + std::to_string(newX) + "," + std::to_string(newY) + ")\n") ;

    if(battlefield->isInside(newX, newY) && !battlefield->isOccupied(newX, newY)) {
        setPosition(newX, newY) ;

        battlefield->getLogger()->log(getName() + " moves to (" + std::to_string(getX()) + ", " + std::to_string(getY()) + ")\n") ;
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

        int targetX = getX() + dx;
        int targetY = getY() + dy;

        if (!battlefield->isInside(targetX, targetY)) {                     //only shoot inside the battlefield area
            battlefield->getLogger()->log(getName() + " tried to fire outside the battlefield.\n") ;
            return;
        }

        battlefield->getLogger()->log(getName() + " fires at (" + std::to_string(targetX) + ", " + std::to_string(targetY) + ")\n") ;

        for (Robot* other : battlefield->getRobots()) {                            //check if there is robot there
            if (other->isAlive() && other->getX() == targetX && other->getY() == targetY) {

                battlefield->getLogger()->log(getName() + " hits " + other->getName() + "!\n") ;
                other->takeDamage();     //other robot take damage
                addUpgradePoints() ;     //this robot get 1 upgrade point

                break; // Only one robot can be at a position
            }
        }
    }
    else {
        battlefield->getLogger()->log(getName() + " is out of shells and self-destructs!\n") ;
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
    int targetX = getX() + dx ;
    int targetY = getY() + dy ;
    std::vector<std::pair<int,int>> lookAreas = {
        {targetX - 1 , targetY - 1}, {targetX , targetY - 1}, {targetX + 1 , targetY - 1},
        {targetX - 1 , targetY}    , {targetX , targetY}    , {targetX + 1 , targetY}    ,
        {targetX - 1 , targetY + 1}, {targetX , targetY + 1}, {targetX + 1 , targetY + 1}
    };
    battlefield->getLogger()->log(getName() + " is looking at (" + std::to_string(targetX) + ", " + std::to_string(targetY) + ")\n") ;

    std::vector<std::pair<int,int>> foundRobot ;

    for(Robot* other : battlefield->getRobots()) {
        for(std::pair<int,int>& lookArea : lookAreas) {
            if(other != this && other->isAlive() && other->getX() == lookArea.first && other->getY() == lookArea.second) {
                battlefield->getLogger()->log(getName() + " found " + other->getName() + " at (" + std::to_string(other->getX()) + "," + std::to_string(other->getY()) + ")\n") ;
                foundRobot.push_back({other->getX() , other->getY()}) ;
            }
        }
    }
}

void ThinkingRobot::think() {
    battlefield->getLogger()->log(getName() + " is thinking about its next move.\n") ;
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
    setLives(1) ;
    setShells(10) ;
    //setUpgradePoints(0) ;  //if reset upgradePoints every time revive, we might never see tier 3 robot
}

bool HideBot::canHide() {
    return remainingHides > 0 ;
}

void HideBot::takeDamage() {
    if(canHide()) {
        remainingHides--;
        battlefield->getLogger()->log(getName() + " is hiding and avoid the hit (invulnerable). Hides left: " + std::to_string(remainingHides) + "\n") ;
        return ;
    }
    else {
        subLives() ;
        battlefield->getLogger()->log(getName() + " tried to hide but has no hides left! TAKING DAMAGE!\n") ;
    }
}

void JumpBot::move(int dx, int dy) {                 //changed from jump() to just overriding move()
    if(canJump()) {
        int newX = rand() % battlefield->getCols();       //random position inside the boundaries
        int newY = rand() % battlefield->getRows();

        if(battlefield->isInside(newX, newY) && !battlefield->isOccupied(newX, newY)) {
            setPosition(newX, newY);
            remainingJumps--;
            battlefield->getLogger()->log(getName() + " jumped to (" + std::to_string(newX) + ", "
                                          + std::to_string(newY) + "). Jumps left: " + std::to_string(remainingJumps) + "\n") ;
        }
        else {
            battlefield->getLogger()->log(getName() + " tried to jump to (" + std::to_string(newX) + "," + std::to_string(newY)
                                          + "). Invalid Position. No Jumps consumed.\n") ;
        }
    }
    else {
        battlefield->getLogger()->log(getName() + " tried to jump but has no jumps left! Proceed with normal movement logic\n") ;

        int newX = getX() + dx;
        int newY = getY() + dy;
        battlefield->getLogger()->log(getName() + " want to move to (" + std::to_string(newX) + "," + std::to_string(newY) + ")\n") ;

        if(battlefield->isInside(newX, newY) && !battlefield->isOccupied(newX, newY)) {
            setPosition(newX , newY) ;
            battlefield->getLogger()->log(getName() + " moves to (" + std::to_string(getX()) + ", " + std::to_string(getY()) + ")\n") ;
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
    int oldY = getY() ;
    int oldX = getX() ;                             //for display purposes

    if(directions[idxDirection] == "up") {
        if(getY() == 0) {
            battlefield->getLogger()->log("At the edge, Cannot move up. Skipping Move.\n") ;
            return ;
        }

        dy = rand() % getY() ;                //valid dy to move

        setPosition(getX() , getY() - dy) ;           //x remain constant, y moving

        battlefield->getLogger()->log(getName() + " is charging through the line from (" + std::to_string(oldX) + "," + std::to_string(oldY) + ") towards ("
                                      + std::to_string(getX()) + "," + std::to_string(getY()) + "). Dealing damage to all robot along the path\n") ;

        for(Robot* other : battlefield->getRobots()) {  //dealing damage along passed line
            for(int i = 1 ; i <= dy ; i++) {
                if(other != this && other->isAlive() && other->getX() == oldX && other->getY() == (oldY - i)) {
                    other->takeDamage() ;
                    addUpgradePoints() ;
                }
            }
        }
    }
    else if(directions[idxDirection] == "down") {
        if(getY() == battlefield->getRows() - 1) {
            battlefield->getLogger()->log("At the edge, Cannot move down. Skipping Move.\n") ;
            return ;
        }

        dy = rand() % (battlefield->getRows() - getY()) ;
        setPosition(getX() , getY() + dy) ;

        battlefield->getLogger()->log(getName() + " is charging through the line from (" + std::to_string(oldX) + "," + std::to_string(oldY) + ") towards ("
                                      + std::to_string(getX()) + "," + std::to_string(getY()) + "). Dealing damage to all robot along the path\n") ;

        for(Robot* other : battlefield->getRobots()) {
            for(int i = 1 ; i <= dy ; i++) {
                if(other != this && other->isAlive() && other->getX() == oldX && other->getY() == (oldY + i)) {
                    other->takeDamage() ;
                    addUpgradePoints() ;
                }
            }
        }
    }
    else if(directions[idxDirection] == "left") {
        if(getX() == 0) {
            battlefield->getLogger()->log("At the edge, Cannot move left. Skipping Move.\n") ;
            return ;
        }


        dx = rand() % (getX() - 0) ;

        setPosition(getX() - dx , getY()) ;           //y remain constact, x moving

        battlefield->getLogger()->log(getName() + " is charging through the line from (" + std::to_string(oldX) + "," + std::to_string(oldY) + ") towards ("
                                      + std::to_string(getX()) + "," + std::to_string(getY()) + "). Dealing damage to all robot along the path\n") ;

        for(Robot* other : battlefield->getRobots()) {
            for(int i = 1 ; i <= dx ; i++) {
                if(other != this && other->isAlive() && other->getX() == (oldX - i) && other->getY() == oldY) {
                    other->takeDamage() ;
                    addUpgradePoints() ;
                }
            }
        }
    }
    else if(directions[idxDirection] == "right") {
        if(getX() == battlefield->getCols() - 1) {
            battlefield->getLogger()->log("At the edge, Cannot move right. Skipping Move.\n") ;
            return ;
        }


        dx = rand() % (battlefield->getCols() - getX()) ;
        setPosition(getX() + dx , getY()) ;

        battlefield->getLogger()->log(getName() + " is charging through the line from (" + std::to_string(oldX) + "," + std::to_string(oldY) + ") towards ("
                                      + std::to_string(getX()) + "," + std::to_string(getY()) + "). Dealing damage to all robot along the path\n") ;

        for(Robot* other : battlefield->getRobots()) {
            for(int i = 1 ; i <= dx ; i++) {
                if(other != this && other->isAlive() && other->getX() == (oldX + i) && other->getY() == oldY) {
                    other->takeDamage() ;
                    addUpgradePoints() ;
                }
            }
        }
    }
}

void TrueDamageBot::fire(int dx, int dy) {
    if (dx == 0 && dy == 0) return;

    if(getShells() > 0) {
        subShells() ;

        int targetX = getX() + dx;
        int targetY = getY() + dy;

        if (!battlefield->isInside(targetX, targetY)) {
            battlefield->getLogger()->log(getName() + " tried to fire outside the battlefield.\n") ;
            return;
        }

        battlefield->getLogger()->log(getName() + " fires at (" + std::to_string(targetX) + ", " + std::to_string(targetY) + ")\n") ;

        for (Robot* other : battlefield->getRobots()) {
            if (other != this && other->isAlive() && other->getX() == targetX && other->getY() == targetY) {

                battlefield->getLogger()->log(name + " hits " + other->getName() + "!\n") ;
                other->takeDamage();         //deal damage as usual
                addUpgradePoints() ;         //get one upgrade point

                if (rand() % 2 == 0) {       //50% chance to deal extra damage
                    battlefield->getLogger()->log("True damage triggered, directly reducing " + other->getName() +  " revivals by 1\n") ;
                    other->subRevivals() ;
                }
                break;
            }
        }
    }
    else {
        battlefield->getLogger()->log(name + " is out of shells and self-destructs!\n") ;
        kill();
    }
}

void LifestealBot::fire(int dx, int dy) {
    if (dx == 0 && dy == 0) return;

    if(getShells() > 0) {
        subShells() ;

        int targetX = getX() + dx;
        int targetY = getY() + dy;

        if (!battlefield->isInside(targetX, targetY)) {
            battlefield->getLogger()->log(name + " tried to fire outside the battlefield.\n") ;
            return;
        }

        battlefield->getLogger()->log(name + " fires at (" + std::to_string(targetX) + ", " + std::to_string(targetY) + ")\n") ;

        for (Robot* other : battlefield->getRobots()) {
            if (other != this && other->isAlive() && other->getX() == targetX && other->getY() == targetY) {

                battlefield->getLogger()->log(name + " hits " + other->getName() + "!\n") ;
                other->takeDamage();         //deal damage as usual
                addUpgradePoints() ;

                if (rand() % 2 == 0) {       //50% chance to absorb live
                    battlefield->getLogger()->log("Lifesteal triggered, " + other->getName() +  " absorbs energy and gains 1 revival point!\n") ;
                    other->addRevivals() ;
                }
                break;
            }
        }
    }
    else {
        battlefield->getLogger()->log(name + " is out of shells and self-destructs!\n") ;
        kill();
    }
}

void LongshotBot::fire(int dx, int dy) {
    do {
        dx = (rand() % 7) - 3;  // range: -3 to 3
        dy = (rand() % 7) - 3;
    } while (std::abs(dx) + std::abs(dy) > 3 || (dx == 0 && dy == 0));  //loop until find valid position 3 units away

    if(getShells() > 0) {
        subShells() ;

        int targetX = getX() + dx;
        int targetY = getY() + dy;

        if (!battlefield->isInside(targetX, targetY)) {
            battlefield->getLogger()->log(name + " tried to fire outside the battlefield.\n") ;
            return;
        }

        battlefield->getLogger()->log(name + " fires at (" + std::to_string(targetX) + ", " + std::to_string(targetY) + ")\n") ;

        for (Robot* other : battlefield->getRobots()) {
            if (other != this && other->isAlive() && other->getX() == targetX && other->getY() == targetY) {

                battlefield->getLogger()->log(name + " hits " + other->getName() + "!\n") ;
                other->takeDamage();         //deal damage as usual
                addUpgradePoints() ;
                break;
            }
        }
    }
    else {
        battlefield->getLogger()->log(name + " is out of shells and self-destructs!\n") ;
        kill();
    }
}

void SemiautoBot::fire(int dx, int dy) {
    if (dx == 0 && dy == 0) return;

    int targetX = getX() + dx;
    int targetY = getY() + dy;

    if (!battlefield->isInside(targetX, targetY)) {
            battlefield->getLogger()->log(name + " want to fire outside the battlefield. Skipping Fire.\n") ;
            return;
    }

    if(getShells() > 2) {
        battlefield->getLogger()->log(name + " perform semi-auto fires at (" + std::to_string(targetX) + ", " + std::to_string(targetY) + ")\n") ;
        subShells() ;
        subShells() ;
        subShells() ;

        for(int i = 1 ; i <= 3 ; i++) {
            if ((rand() % 100) < 70) {                          // 70% hit chance
                for (Robot* other : battlefield->getRobots()) {
                    if (other != this && other->isAlive() && other->getX() == targetX && other->getY() == targetY) {

                        battlefield->getLogger()->log(name + "'s shot #" + std::to_string(i) + " hits " + other->getName() + "!\n");
                        other->takeDamage();
                        addUpgradePoints() ;
                        break ;
                    }
                }
            }
            else {
                battlefield->getLogger()->log(name + "'s shot #" + std::to_string(i) + " hit nothing.\n");
            }
        }

    }
    else if(getShells() > 0) {
        subShells() ;
        battlefield->getLogger()->log(name + " is low on shells, switching to normal shooting.\n") ;
        battlefield->getLogger()->log(name + " fires at (" + std::to_string(targetX) + ", " + std::to_string(targetY) + ")\n") ;

        for (Robot* other : battlefield->getRobots()) {
            if (other != this && other->isAlive() && other->getX() == targetX && other->getY() == targetY) {

                battlefield->getLogger()->log(name + " hits " + other->getName() + "!\n") ;
                other->takeDamage();         //deal damage as usual
                addUpgradePoints() ;
                break;
            }
        }
    }
    else {
        battlefield->getLogger()->log(name + " is out of shells and self-destructs!\n") ;
        kill();
    }
}

void ScoutBot::look(int dx, int dy) {

    battlefield->getLogger()->log(name + " is looking at the entire battlefield.\n") ;

    std::vector<std::pair<int,int>> foundRobot ;

    for(Robot* other : battlefield->getRobots()) {
        if(other != this && other->isAlive()) {
            battlefield->getLogger()->log(name + " found " + other->getName() + " at (" + std::to_string(other->getX()) + "," + std::to_string(other->getY()) + ")\n") ;
            foundRobot.push_back({other->getX() , other->getY()}) ;
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

        reviveOne() ;                         //try to revive one robot from the queue

        for(Robot* robot : robots) {         //each robot take turn
            if (robot->isAlive()) {
                robot->takeTurn();
            }
        }

        for(Robot* robot : robots) {         //find ded robot and send them to graveyard queue
            if(!robot->isAlive()) {
                if (std::find(graveyard.begin(), graveyard.end(), robot) == graveyard.end()) {   //check if the robot is already waiting inside the queue
                    getLogger()->log(robot->getName() + " is ded. Sent to graveyard.\n") ;
                    enterGraveyard(robot) ;
                }

            }
        }

        for(Robot* robot : robots) {          //upgrade all robot that can be upgrade
            if (robot->isAlive() && (!robot->getUpgradeFirst() || !robot->getUpgradeSecond() || !robot->getUpgradeThird()) && (robot->getUpgradePoints() > 0)) {
                upgrade(robot);
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
    graveyard.push_back(robot) ;
}

void Battlefield::reviveOne() {
    if (!graveyard.empty()) {
        Robot* deadRobot = graveyard.front();

        if(deadRobot->canRevive()) {       //create a new copy of the object then destroy the previous one. because upgraded robot need to degrade back into genericrobot
            GenericRobot* revivedRobot ;
            int newX ;
            int newY ;
            do {                 //loop eternally until we can get unoccupied space
                newX = rand() % cols;
                newY = rand() % rows;

                if (!isOccupied(newX, newY)) {
                    deadRobot->subRevivals() ;

                    revivedRobot = new GenericRobot("GenericRobot", deadRobot->getName(), newX, newY, this) ;
                    revivedRobot->setRevivals(deadRobot->getRevivals()) ;
                    revivedRobot->reset() ;
                    createRobot(revivedRobot) ;

                    graveyard.erase(graveyard.begin());  //kick out of the queue
                    robots.erase(std::remove(robots.begin(), robots.end(), deadRobot), robots.end());  //kick out of robots vector
                    delete deadRobot ;

                    getLogger()->log(revivedRobot->getName() + " has been revived at (" + std::to_string(newX) + "," + std::to_string(newY) + ")"
                                     + ". Remaining revivals : " + std::to_string(revivedRobot->getRevivals()) + "\n") ;


                    return ;
                }
            } while(isOccupied(newX, newY)) ;
        }
        else {
            getLogger()->log("Attempting to revive " + deadRobot->getName() + " but no revives left. let him ascend.\n") ;
            graveyard.erase(graveyard.begin());      //if cannot revive just kick out of the queue
            robots.erase(std::remove(robots.begin(), robots.end(), deadRobot), robots.end());  //kick out of the graveyard
            delete deadRobot ;                       //destroy his soul
        }
    }
}

void Battlefield::upgrade(Robot* robot) {
    if(robot->getType() == "GenericRobot") {      //first upgrade
        int choice = rand() % 3;                  //0 HideBot, 1 JumpBot, 2 JuggernautBot

        GenericRobot* upgradedRobot ;
        if (choice == 0) {
            // Upgrade to HideBot
            upgradedRobot = new HideBot("HideBot", robot->getName(), robot->getX(), robot->getY(), this);
        }
        else if (choice == 1) {
            // Upgrade to JumpBot
            upgradedRobot = new JumpBot("JumpBot", robot->getName(), robot->getX(), robot->getY(), this);
        }
        else {
            upgradedRobot = new JuggernautBot("JuggernautBot", robot->getName(), robot->getX(), robot->getY(), this);
        }

        upgradedRobot->setRevivals(robot->getRevivals()) ;               //copy the revivals
        upgradedRobot->setUpgradePoints(robot->getUpgradePoints() - 1) ; //copy the upgradePoints minus one
        createRobot(upgradedRobot) ;

        getLogger()->log(upgradedRobot->getName() + " has been upgraded into " + upgradedRobot->getType() + "\n") ;

        robots.erase(std::remove(robots.begin(), robots.end(), robot), robots.end());  //remove the beforeUpgradeState from the vector
        delete robot ;
        return ;
    }
    else if(robot->getType() == "HideBot") {      //for second upgrades
        int choice = rand() % 5;                  //0 Longshot, 1 Semiauto, 2 Thirtyshot, 3 truedamage, 4 lifesteal

        HideBot* upgradedRobot ;
        if (choice == 0) {
            // Upgrade to HideBot
            upgradedRobot = new HideLongshotBot("HideLongshotBot", robot->getName(), robot->getX(), robot->getY(), this);
        }
        else if (choice == 1) {
            // Upgrade to JumpBot
            upgradedRobot = new HideSemiautoBot("HideSemiautoBot", robot->getName(), robot->getX(), robot->getY(), this);
        }
        else if (choice == 2) {
            upgradedRobot = new HideThirtyshotBot("HideThirtyshotBot", robot->getName(), robot->getX(), robot->getY(), this);
        }
        else if (choice == 3) {
            upgradedRobot = new HideTrueDamageBot("HideTrueDamageBot", robot->getName(), robot->getX(), robot->getY(), this);
        }
        else if (choice == 4) {
            upgradedRobot = new HideLifestealBot("HideLifestealBot", robot->getName(), robot->getX(), robot->getY(), this);
        }

        upgradedRobot->setRevivals(robot->getRevivals()) ;  //copy the remainingRevivals
        upgradedRobot->setUpgradePoints(robot->getUpgradePoints() - 1) ; //copy the upgradePoints minus one
        createRobot(upgradedRobot) ;

        getLogger()->log(upgradedRobot->getName() + " has been upgraded into " + upgradedRobot->getType() + "\n") ;

        robots.erase(std::remove(robots.begin(), robots.end(), robot), robots.end());  //remove the beforeUpgradeState from the vector
        delete robot ;
        return ;
    }
    else if(robot->getType() == "JumpBot") {
        int choice = rand() % 5;                  //0 Longshot, 1 Semiauto, 2 Thirtyshot, 3 truedamage, 4 lifesteal

        JumpBot* upgradedRobot ;
        if (choice == 0) {
            // Upgrade to HideBot
            upgradedRobot = new JumpLongshotBot("JumpLongshotBot", robot->getName(), robot->getX(), robot->getY(), this);
        }
        else if (choice == 1) {
            // Upgrade to JumpBot
            upgradedRobot = new JumpSemiautoBot("JumpSemiautoBot", robot->getName(), robot->getX(), robot->getY(), this);
        }
        else if (choice == 2) {
            upgradedRobot = new JumpThirtyshotBot("JumpThirtyshotBot", robot->getName(), robot->getX(), robot->getY(), this);
        }
        else if (choice == 3) {
            upgradedRobot = new JumpTrueDamageBot("JumpTrueDamageBot", robot->getName(), robot->getX(), robot->getY(), this);
        }
        else if (choice == 4) {
            upgradedRobot = new JumpLifestealBot("JumpLifestealBot", robot->getName(), robot->getX(), robot->getY(), this);
        }

        upgradedRobot->setRevivals(robot->getRevivals()) ;  //copy the remainingRevivals
        upgradedRobot->setUpgradePoints(robot->getUpgradePoints() - 1) ; //copy the upgradePoints minus one
        createRobot(upgradedRobot) ;

        getLogger()->log(upgradedRobot->getName() + " has been upgraded into " + upgradedRobot->getType() + "\n") ;

        robots.erase(std::remove(robots.begin(), robots.end(), robot), robots.end());  //remove the beforeUpgradeState from the vector
        delete robot ;
        return ;
    }
    else if(robot->getType() == "JuggernautBot") {
        int choice = rand() % 5;                  //0 Longshot, 1 Semiauto, 2 Thirtyshot, 3 truedamage, 4 lifesteal

        JuggernautBot* upgradedRobot ;
        if (choice == 0) {
            // Upgrade to HideBot
            upgradedRobot = new JuggernautLongshotBot("JuggernautLongshotBot", robot->getName(), robot->getX(), robot->getY(), this);
        }
        else if (choice == 1) {
            // Upgrade to JumpBot
            upgradedRobot = new JuggernautSemiautoBot("JuggernautSemiautoBot", robot->getName(), robot->getX(), robot->getY(), this);
        }
        else if (choice == 2) {
            upgradedRobot = new JuggernautThirtyshotBot("JuggernautThirtyshotBot", robot->getName(), robot->getX(), robot->getY(), this);
        }
        else if (choice == 3) {
            upgradedRobot = new JuggernautTrueDamageBot("JuggernautTrueDamageBot", robot->getName(), robot->getX(), robot->getY(), this);
        }
        else if (choice == 4) {
            upgradedRobot = new JuggernautLifestealBot("JuggernautLifestealBot", robot->getName(), robot->getX(), robot->getY(), this);
        }

        upgradedRobot->setRevivals(robot->getRevivals()) ;  //copy the remainingRevivals
        upgradedRobot->setUpgradePoints(robot->getUpgradePoints() - 1) ; //copy the upgradePoints minus one
        createRobot(upgradedRobot) ;

        getLogger()->log(upgradedRobot->getName() + " has been upgraded into " + upgradedRobot->getType() + "\n") ;

        robots.erase(std::remove(robots.begin(), robots.end(), robot), robots.end());  //remove the beforeUpgradeState from the vector
        delete robot ;
        return ;
    }
    else if(robot->getType() == "HideLongshotBot") {   //third upgrade
        int choice = rand() % 2;                  //0 scout, 1 tracker

        HideLongshotBot* upgradedRobot ;
        if (choice == 0) {
            // Upgrade to HideBot
            upgradedRobot = new HideLongshotScoutBot("HideLongshotScoutBot", robot->getName(), robot->getX(), robot->getY(), this);
        }
        else if (choice == 1) {
            // Upgrade to JumpBot
            upgradedRobot = new HideLongshotTrackerBot("HideLongshotTrackerBot", robot->getName(), robot->getX(), robot->getY(), this);
        }

        upgradedRobot->setRevivals(robot->getRevivals()) ;  //copy the remainingRevivals
        upgradedRobot->setUpgradePoints(robot->getUpgradePoints() - 1) ; //copy the upgradePoints minus one
        createRobot(upgradedRobot) ;

        getLogger()->log(upgradedRobot->getName() + " has been upgraded into " + upgradedRobot->getType() + "\n") ;

        robots.erase(std::remove(robots.begin(), robots.end(), robot), robots.end());  //remove the beforeUpgradeState from the vector
        delete robot ;
        return ;
    }
    else if(robot->getType() == "HideSemiautoBot") {
        int choice = rand() % 2;                  //0 scout, 1 tracker

        HideSemiautoBot* upgradedRobot ;
        if (choice == 0) {
            // Upgrade to HideBot
            upgradedRobot = new HideSemiautoScoutBot("HideSemiautoScoutBot", robot->getName(), robot->getX(), robot->getY(), this);
        }
        else if (choice == 1) {
            // Upgrade to JumpBot
            upgradedRobot = new HideSemiautoTrackerBot("HideSemiautoTrackerBot", robot->getName(), robot->getX(), robot->getY(), this);
        }

        upgradedRobot->setRevivals(robot->getRevivals()) ;  //copy the remainingRevivals
        upgradedRobot->setUpgradePoints(robot->getUpgradePoints() - 1) ; //copy the upgradePoints minus one
        createRobot(upgradedRobot) ;

        getLogger()->log(upgradedRobot->getName() + " has been upgraded into " + upgradedRobot->getType() + "\n") ;

        robots.erase(std::remove(robots.begin(), robots.end(), robot), robots.end());  //remove the beforeUpgradeState from the vector
        delete robot ;
        return ;
    }
    else if(robot->getType() == "HideThirtyshotBot") {
        int choice = rand() % 2;                  //0 scout, 1 tracker

        HideThirtyshotBot* upgradedRobot ;
        if (choice == 0) {
            // Upgrade to HideBot
            upgradedRobot = new HideThirtyshotScoutBot("HideThirtyshotScoutBot", robot->getName(), robot->getX(), robot->getY(), this);
        }
        else if (choice == 1) {
            // Upgrade to JumpBot
            upgradedRobot = new HideThirtyshotTrackerBot("HideThirtyshotTrackerBot", robot->getName(), robot->getX(), robot->getY(), this);
        }

        upgradedRobot->setRevivals(robot->getRevivals()) ;  //copy the remainingRevivals
        upgradedRobot->setUpgradePoints(robot->getUpgradePoints() - 1) ; //copy the upgradePoints minus one
        createRobot(upgradedRobot) ;

        getLogger()->log(upgradedRobot->getName() + " has been upgraded into " + upgradedRobot->getType() + "\n") ;

        robots.erase(std::remove(robots.begin(), robots.end(), robot), robots.end());  //remove the beforeUpgradeState from the vector
        delete robot ;
        return ;
    }
    else if(robot->getType() == "HideTrueDamageBot") {
        int choice = rand() % 2;                  //0 scout, 1 tracker

        HideTrueDamageBot* upgradedRobot ;
        if (choice == 0) {
            // Upgrade to HideBot
            upgradedRobot = new HideTrueDamageScoutBot("HideTrueDamageScoutBot", robot->getName(), robot->getX(), robot->getY(), this);
        }
        else if (choice == 1) {
            // Upgrade to JumpBot
            upgradedRobot = new HideTrueDamageTrackerBot("HideTrueDamageTrackerBot", robot->getName(), robot->getX(), robot->getY(), this);
        }

        upgradedRobot->setRevivals(robot->getRevivals()) ;  //copy the remainingRevivals
        upgradedRobot->setUpgradePoints(robot->getUpgradePoints() - 1) ; //copy the upgradePoints minus one
        createRobot(upgradedRobot) ;

        getLogger()->log(upgradedRobot->getName() + " has been upgraded into " + upgradedRobot->getType() + "\n") ;

        robots.erase(std::remove(robots.begin(), robots.end(), robot), robots.end());  //remove the beforeUpgradeState from the vector
        delete robot ;
        return ;
    }
    else if(robot->getType() == "HideLifestealBot") {
        int choice = rand() % 2;                  //0 scout, 1 tracker

        HideLifestealBot* upgradedRobot ;
        if (choice == 0) {
            // Upgrade to HideBot
            upgradedRobot = new HideLifestealScoutBot("HideLifestealScoutBot", robot->getName(), robot->getX(), robot->getY(), this);
        }
        else if (choice == 1) {
            // Upgrade to JumpBot
            upgradedRobot = new HideLifestealTrackerBot("HideLifestealTrackerBot", robot->getName(), robot->getX(), robot->getY(), this);
        }

        upgradedRobot->setRevivals(robot->getRevivals()) ;  //copy the remainingRevivals
        upgradedRobot->setUpgradePoints(robot->getUpgradePoints() - 1) ; //copy the upgradePoints minus one
        createRobot(upgradedRobot) ;

        getLogger()->log(upgradedRobot->getName() + " has been upgraded into " + upgradedRobot->getType() + "\n") ;

        robots.erase(std::remove(robots.begin(), robots.end(), robot), robots.end());  //remove the beforeUpgradeState from the vector
        delete robot ;
        return ;
    }
    else if(robot->getType() == "JumpLongshotBot") {
        int choice = rand() % 2;                  //0 scout, 1 tracker

        JumpLongshotBot* upgradedRobot ;
        if (choice == 0) {
            // Upgrade to HideBot
            upgradedRobot = new JumpLongshotScoutBot("JumpLongshotScoutBot", robot->getName(), robot->getX(), robot->getY(), this);
        }
        else if (choice == 1) {
            // Upgrade to JumpBot
            upgradedRobot = new JumpLongshotTrackerBot("JumpLongshotTrackerBot", robot->getName(), robot->getX(), robot->getY(), this);
        }

        upgradedRobot->setRevivals(robot->getRevivals()) ;  //copy the remainingRevivals
        upgradedRobot->setUpgradePoints(robot->getUpgradePoints() - 1) ; //copy the upgradePoints minus one
        createRobot(upgradedRobot) ;

        getLogger()->log(upgradedRobot->getName() + " has been upgraded into " + upgradedRobot->getType() + "\n") ;

        robots.erase(std::remove(robots.begin(), robots.end(), robot), robots.end());  //remove the beforeUpgradeState from the vector
        delete robot ;
        return ;
    }
    else if(robot->getType() == "JumpSemiautoBot") {
        int choice = rand() % 2;                  //0 scout, 1 tracker

        JumpSemiautoBot* upgradedRobot ;
        if (choice == 0) {
            // Upgrade to HideBot
            upgradedRobot = new JumpSemiautoScoutBot("JumpSemiautoScoutBot", robot->getName(), robot->getX(), robot->getY(), this);
        }
        else if (choice == 1) {
            // Upgrade to JumpBot
            upgradedRobot = new JumpSemiautoTrackerBot("JumpSemiautoTrackerBot", robot->getName(), robot->getX(), robot->getY(), this);
        }

        upgradedRobot->setRevivals(robot->getRevivals()) ;  //copy the remainingRevivals
        upgradedRobot->setUpgradePoints(robot->getUpgradePoints() - 1) ; //copy the upgradePoints minus one
        createRobot(upgradedRobot) ;

        getLogger()->log(upgradedRobot->getName() + " has been upgraded into " + upgradedRobot->getType() + "\n") ;

        robots.erase(std::remove(robots.begin(), robots.end(), robot), robots.end());  //remove the beforeUpgradeState from the vector
        delete robot ;
        return ;
    }
    else if(robot->getType() == "JumpThirtyshotBot") {
        int choice = rand() % 2;                  //0 scout, 1 tracker

        JumpThirtyshotBot* upgradedRobot ;
        if (choice == 0) {
            // Upgrade to HideBot
            upgradedRobot = new JumpThirtyshotScoutBot("JumpThirtyshotScoutBot", robot->getName(), robot->getX(), robot->getY(), this);
        }
        else if (choice == 1) {
            // Upgrade to JumpBot
            upgradedRobot = new JumpThirtyshotTrackerBot("JumpThirtyshotTrackerBot", robot->getName(), robot->getX(), robot->getY(), this);
        }

        upgradedRobot->setRevivals(robot->getRevivals()) ;  //copy the remainingRevivals
        upgradedRobot->setUpgradePoints(robot->getUpgradePoints() - 1) ; //copy the upgradePoints minus one
        createRobot(upgradedRobot) ;

        getLogger()->log(upgradedRobot->getName() + " has been upgraded into " + upgradedRobot->getType() + "\n") ;

        robots.erase(std::remove(robots.begin(), robots.end(), robot), robots.end());  //remove the beforeUpgradeState from the vector
        delete robot ;
        return ;
    }
    else if(robot->getType() == "JumpTrueDamageBot") {
        int choice = rand() % 2;                  //0 scout, 1 tracker

        JumpTrueDamageBot* upgradedRobot ;
        if (choice == 0) {
            // Upgrade to HideBot
            upgradedRobot = new JumpTrueDamageScoutBot("JumpTrueDamageScoutBot", robot->getName(), robot->getX(), robot->getY(), this);
        }
        else if (choice == 1) {
            // Upgrade to JumpBot
            upgradedRobot = new JumpTrueDamageTrackerBot("JumpTrueDamageTrackerBot", robot->getName(), robot->getX(), robot->getY(), this);
        }

        upgradedRobot->setRevivals(robot->getRevivals()) ;  //copy the remainingRevivals
        upgradedRobot->setUpgradePoints(robot->getUpgradePoints() - 1) ; //copy the upgradePoints minus one
        createRobot(upgradedRobot) ;

        getLogger()->log(upgradedRobot->getName() + " has been upgraded into " + upgradedRobot->getType() + "\n") ;

        robots.erase(std::remove(robots.begin(), robots.end(), robot), robots.end());  //remove the beforeUpgradeState from the vector
        delete robot ;
        return ;
    }
    else if(robot->getType() == "JumpLifestealBot") {
        int choice = rand() % 2;                  //0 scout, 1 tracker

        JumpLifestealBot* upgradedRobot ;
        if (choice == 0) {
            // Upgrade to HideBot
            upgradedRobot = new JumpLifestealScoutBot("JumpLifestealScoutBot", robot->getName(), robot->getX(), robot->getY(), this);
        }
        else if (choice == 1) {
            // Upgrade to JumpBot
            upgradedRobot = new JumpLifestealTrackerBot("JumpLifestealTrackerBot", robot->getName(), robot->getX(), robot->getY(), this);
        }

        upgradedRobot->setRevivals(robot->getRevivals()) ;  //copy the remainingRevivals
        upgradedRobot->setUpgradePoints(robot->getUpgradePoints() - 1) ; //copy the upgradePoints minus one
        createRobot(upgradedRobot) ;

        getLogger()->log(upgradedRobot->getName() + " has been upgraded into " + upgradedRobot->getType() + "\n") ;

        robots.erase(std::remove(robots.begin(), robots.end(), robot), robots.end());  //remove the beforeUpgradeState from the vector
        delete robot ;
        return ;
    }
    else if(robot->getType() == "JuggernautLongshotBot") {
        int choice = rand() % 2;                  //0 scout, 1 tracker

        JuggernautLongshotBot* upgradedRobot ;
        if (choice == 0) {
            // Upgrade to HideBot
            upgradedRobot = new JuggernautLongshotScoutBot("JuggernautLongshotScoutBot", robot->getName(), robot->getX(), robot->getY(), this);
        }
        else if (choice == 1) {
            // Upgrade to JumpBot
            upgradedRobot = new JuggernautLongshotTrackerBot("JuggernautLongshotTrackerBot", robot->getName(), robot->getX(), robot->getY(), this);
        }

        upgradedRobot->setRevivals(robot->getRevivals()) ;  //copy the remainingRevivals
        upgradedRobot->setUpgradePoints(robot->getUpgradePoints() - 1) ; //copy the upgradePoints minus one
        createRobot(upgradedRobot) ;

        getLogger()->log(upgradedRobot->getName() + " has been upgraded into " + upgradedRobot->getType() + "\n") ;

        robots.erase(std::remove(robots.begin(), robots.end(), robot), robots.end());  //remove the beforeUpgradeState from the vector
        delete robot ;
        return ;
    }
    else if(robot->getType() == "JuggernautSemiautoBot") {
        int choice = rand() % 2;                  //0 scout, 1 tracker

        JuggernautSemiautoBot* upgradedRobot ;
        if (choice == 0) {
            // Upgrade to HideBot
            upgradedRobot = new JuggernautSemiautoScoutBot("JuggernautSemiautoScoutBot", robot->getName(), robot->getX(), robot->getY(), this);
        }
        else if (choice == 1) {
            // Upgrade to JumpBot
            upgradedRobot = new JuggernautSemiautoTrackerBot("JuggernautSemiautoTrackerBot", robot->getName(), robot->getX(), robot->getY(), this);
        }

        upgradedRobot->setRevivals(robot->getRevivals()) ;  //copy the remainingRevivals
        upgradedRobot->setUpgradePoints(robot->getUpgradePoints() - 1) ; //copy the upgradePoints minus one
        createRobot(upgradedRobot) ;

        getLogger()->log(upgradedRobot->getName() + " has been upgraded into " + upgradedRobot->getType() + "\n") ;

        robots.erase(std::remove(robots.begin(), robots.end(), robot), robots.end());  //remove the beforeUpgradeState from the vector
        delete robot ;
        return ;
    }
    else if(robot->getType() == "JuggernautThirtyshotBot") {
        int choice = rand() % 2;                  //0 scout, 1 tracker

        JuggernautThirtyshotBot* upgradedRobot ;
        if (choice == 0) {
            // Upgrade to HideBot
            upgradedRobot = new JuggernautThirtyshotScoutBot("JuggernautThirtyshotScoutBot", robot->getName(), robot->getX(), robot->getY(), this);
        }
        else if (choice == 1) {
            // Upgrade to JumpBot
            upgradedRobot = new JuggernautThirtyshotTrackerBot("JuggernautThirtyshotTrackerBot", robot->getName(), robot->getX(), robot->getY(), this);
        }

        upgradedRobot->setRevivals(robot->getRevivals()) ;  //copy the remainingRevivals
        upgradedRobot->setUpgradePoints(robot->getUpgradePoints() - 1) ; //copy the upgradePoints minus one
        createRobot(upgradedRobot) ;

        getLogger()->log(upgradedRobot->getName() + " has been upgraded into " + upgradedRobot->getType() + "\n") ;

        robots.erase(std::remove(robots.begin(), robots.end(), robot), robots.end());  //remove the beforeUpgradeState from the vector
        delete robot ;
        return ;
    }
    else if(robot->getType() == "JuggernautTrueDamageBot") {
        int choice = rand() % 2;                  //0 scout, 1 tracker

        JuggernautTrueDamageBot* upgradedRobot ;
        if (choice == 0) {
            // Upgrade to HideBot
            upgradedRobot = new JuggernautTrueDamageScoutBot("JuggernautTrueDamageScoutBot", robot->getName(), robot->getX(), robot->getY(), this);
        }
        else if (choice == 1) {
            // Upgrade to JumpBot
            upgradedRobot = new JuggernautTrueDamageTrackerBot("JuggernautTrueDamageTrackerBot", robot->getName(), robot->getX(), robot->getY(), this);
        }

        upgradedRobot->setRevivals(robot->getRevivals()) ;  //copy the remainingRevivals
        upgradedRobot->setUpgradePoints(robot->getUpgradePoints() - 1) ; //copy the upgradePoints minus one
        createRobot(upgradedRobot) ;

        getLogger()->log(upgradedRobot->getName() + " has been upgraded into " + upgradedRobot->getType() + "\n") ;

        robots.erase(std::remove(robots.begin(), robots.end(), robot), robots.end());  //remove the beforeUpgradeState from the vector
        delete robot ;
        return ;
    }
    else if(robot->getType() == "JuggernautLifestealBot") {
        int choice = rand() % 2;                  //0 scout, 1 tracker

        JuggernautLifestealBot* upgradedRobot ;
        if (choice == 0) {
            // Upgrade to HideBot
            upgradedRobot = new JuggernautLifestealScoutBot("JuggernautLifestealScoutBot", robot->getName(), robot->getX(), robot->getY(), this);
        }
        else if (choice == 1) {
            // Upgrade to JumpBot
            upgradedRobot = new JuggernautLifestealTrackerBot("JuggernautLifestealTrackerBot", robot->getName(), robot->getX(), robot->getY(), this);
        }

        upgradedRobot->setRevivals(robot->getRevivals()) ;  //copy the remainingRevivals
        upgradedRobot->setUpgradePoints(robot->getUpgradePoints() - 1) ; //copy the upgradePoints minus one
        createRobot(upgradedRobot) ;

        getLogger()->log(upgradedRobot->getName() + " has been upgraded into " + upgradedRobot->getType() + "\n") ;

        robots.erase(std::remove(robots.begin(), robots.end(), robot), robots.end());  //remove the beforeUpgradeState from the vector
        delete robot ;
        return ;
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
