const gba = require('gba');

const SCREEN_WIDTH = 240;
const SCREEN_HEIGHT = 160;
const BALL_RADIUS = 3;
const PADDLE_SPEED = 5;
const PADDLE_HEIGHT = 25;
const PADDLE_WIDTH = 5;

let restart = true;
let atTitleScreen = true;

function showTitleScreen() {
    gba.drawRect(50, 10, 55, 40, gba.colors.BLACK);
    gba.drawRect(55, 10, 70, 15, gba.colors.BLACK);
    gba.drawRect(70, 10, 75, 25, gba.colors.BLACK);
    gba.drawRect(55, 20, 70, 25, gba.colors.BLACK);
    gba.drawRect(80, 10, 85, 25, gba.colors.BLACK);
    gba.drawRect(100, 10, 105, 25, gba.colors.BLACK);
    gba.drawRect(80, 10, 105, 15, gba.colors.BLACK);
    gba.drawRect(80, 20, 105, 25, gba.colors.BLACK);
    gba.drawRect(110, 10, 115, 25, gba.colors.BLACK);
    gba.drawRect(130, 10, 135, 25, gba.colors.BLACK);
    gba.drawRect(110, 10, 135, 15, gba.colors.BLACK);
    gba.drawRect(140, 10, 145, 25, gba.colors.BLACK);
    gba.drawRect(145, 10, 160, 15, gba.colors.BLACK);
    gba.drawRect(160, 10, 165, 40, gba.colors.BLACK);
    gba.drawRect(145, 20, 160, 25, gba.colors.BLACK);
    gba.drawRect(140, 35, 160, 40, gba.colors.BLACK);

    gba.print(5, 50, 'You are the player on the left with');
    gba.print(5, 60, 'the blue paddle. The computer is on');
    gba.print(5, 70, 'the right with the red paddle. Your');
    gba.print(5, 80, 'goal is to bounce the ball past the');
    gba.print(5, 90, `computer's paddle, scoring a point.`);
    gba.print(5, 100, 'Beware! The computer will also try to');
    gba.print(5, 110, 'bounce the ball past your paddle!');
    gba.print(10, 140, gba.colors.RED, 'Press the Start button to begin.');
}

function checkCollisions() {
    if (ball.x >= SCREEN_WIDTH) {
        player.score++;
        restart = true;
        return;
    }

    if (ball.x <= 0) {
        computer.score++;
        restart = true;
        return;
    }

    if (ball.y - BALL_RADIUS <= 0 || ball.y + BALL_RADIUS >= SCREEN_HEIGHT) {
        ball.dy *= -1;
        return;
    }

    if ((ball.y + BALL_RADIUS == player.y ||
         ball.y - BALL_RADIUS == player.y + PADDLE_HEIGHT) &&
        (ball.x >= player.x && ball.x <= player.x + PADDLE_WIDTH)) {
            ball.dy *= -1;
            return;
    }

    if ((ball.y + BALL_RADIUS == computer.y ||
         ball.y - BALL_RADIUS == computer.y + PADDLE_HEIGHT) &&
        (ball.x >= computer.x && ball.x <= computer.x + PADDLE_WIDTH)) {
            ball.dy *= -1;
            return;
    }

    if (ball.x - BALL_RADIUS == player.x &&
        (ball.y >= player.y && ball.y <= player.y + PADDLE_HEIGHT)) {
            ball.dx *= -1;
            return;
    }

    if (ball.x + BALL_RADIUS == computer.x &&
        (ball.y >= computer.y && ball.y <= computer.y + PADDLE_HEIGHT)) {
            ball.dx *= -1;
            return;
    }
}

const player = {
    x: 25, y: 60, score: 0
};
const computer = {
    x: 215, y: 60, score: 0
};
const ball = {
    x: 0, y: 0, dx: 0, dy: 0
};

function draw() {
    gba.drawRect(player.x - PADDLE_WIDTH, player.y + PADDLE_HEIGHT, player.x, player.y,
        gba.colors.BLUE);
    gba.drawRect(computer.x, computer.y, computer.x + PADDLE_WIDTH, computer.y +
        PADDLE_HEIGHT, gba.colors.RED);
    gba.drawCircle(ball.x, ball.y, BALL_RADIUS, gba.colors.BLACK);
    gba.print(0, SCREEN_HEIGHT - 10, gba.colors.BLUE, `Score: ${player.score}`);
    gba.print(SCREEN_WIDTH - 32, SCREEN_HEIGHT - 10, gba.colors.RED,
        `Score: ${computer.score}`);
}

gba.onTick(() => {
    if (atTitleScreen) {
        showTitleScreen();
        if (gba.isKeyDown(gba.keys.KEY_START)) {
            atTitleScreen = false;
        }

        return;
    }

    if (restart) {
        restart = false;
        ball.x = SCREEN_WIDTH / 2;
        ball.y = SCREEN_HEIGHT / 2;
        ball.dx = gba.getRandom() % 3 + 1;
        ball.dy = gba.getRandom() % 3 + 1;
        ball.dx *= (gba.getRandom() % 2) ? 1 : -1;
        ball.dy *= (gba.getRandom() % 2) ? 1 : -1;
    }

    let direction = 0;
    if (gba.isKeyDown(gba.keys.KEY_UP)) {
        direction = -1;
    } else if (gba.isKeyDown(gba.keys.KEY_DOWN)) {
        direction = 1;
    }

    player.y += PADDLE_SPEED * direction;
    computer.y = (ball.y + 10) * 0.75;
    ball.x += ball.dx;
    ball.y += ball.dy;

    checkCollisions();
    draw();
});
