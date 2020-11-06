from sanic import Blueprint
from sanic import response
from sanic.response import json
import asyncio
import random

bp = Blueprint("routes")
bp.static("/", "./server/static")


@bp.route("/")
async def hello(request):
    return await response.file("./server/static/index.html")


@bp.websocket("/random")
async def snapshot(request, socket):
    while not socket.closed:
        message = str(random.randint(0, 100))
        await socket.send(message)
        await asyncio.sleep(2)
