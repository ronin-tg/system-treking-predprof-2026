import socket
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation

UDP_IP = "0.0.0.0"
UDP_PORT = 12345
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind((UDP_IP, UDP_PORT))
sock.setblocking(False)

history = 100
ax_data, ay_data, az_data = [0]*history, [0]*history, [0]*history
gx_data, gy_data, gz_data = [0]*history, [0]*history, [0]*history

plt.style.use('dark_background') 
fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(10, 8), facecolor='#2b2b2b')
fig.canvas.manager.set_window_title('Испытание №1: Телеметрия ПМ')

def setup_plot(axis, title):
    axis.set_facecolor('white') 
    axis.grid(True, color='lightgray', linestyle='-', linewidth=0.5)
    axis.set_title(title, color='white', fontsize=12, pad=10)
    axis.tick_params(colors='white')
    for spine in axis.spines.values():
        spine.set_color('gray')

def update(frame):
    try:
        data, _ = sock.recvfrom(1024)
        v = [float(x) for x in data.decode().split(',')]
        
        for d, val in zip([ax_data, ay_data, az_data, gx_data, gy_data, gz_data], v):
            d.append(val)
            d.pop(0)
            
        ax1.clear()
        setup_plot(ax1, "Данные акселерометра (Линейное ускорение)")
        ax1.plot(ax_data, color='red', label='X', linewidth=1)
        ax1.plot(ay_data, color='green', label='Y', linewidth=1)
        ax1.plot(az_data, color='blue', label='Z', linewidth=1)
        ax1.set_ylim(-17000, 17000)

        ax2.clear()
        setup_plot(ax2, "Данные гироскопа (Угловая скорость)")
        ax2.plot(gx_data, color='orange', label='GX', linewidth=1)
        ax2.plot(gy_data, color='purple', label='GY', linewidth=1)
        ax2.plot(gz_data, color='black', label='GZ', linewidth=1)
        ax2.set_ylim(-20000, 20000)
        
    except:
        pass

setup_plot(ax1, "Данные акселерометра")
setup_plot(ax2, "Данные гироскопа")

print(f"Ожидание данных на порту {UDP_PORT}...")
ani = FuncAnimation(fig, update, interval=30, cache_frame_data=False)
plt.tight_layout(pad=3.0)
plt.show()
