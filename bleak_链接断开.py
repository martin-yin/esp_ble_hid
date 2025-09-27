import asyncio
from bleak import BleakClient, BleakScanner
import logging

address = "10:51:DB:84:4D:EE"  
SERVICE_UUID = "24ed2a4a-643b-4581-87b7-b4d2777de046"
CHARACTERISTIC_UUID = "c34ae534-f289-45b2-b262-f6f415408d65"  

SLEEP_TIME = 5.0
logging.basicConfig(level=logging.INFO)


def disconnect_callback(client):
    print(f"设备 {client.address} 已断开连接")


async def send_command(client, command):
    """向GATT特征写入命令"""
    try:
        command_bytes = command.encode("utf-8") 
        await client.write_gatt_char(CHARACTERISTIC_UUID, command_bytes, response=False)
        print(f"已发送命令: {command}")
    except Exception as e:
        print(f"发送命令失败: {e}")


async def _connect():
    attempt = 0
    max_attempts = 2

    print("开始搜索设备...")
    device = await BleakScanner.find_device_by_address(address)
    if not device:
        print(f"未找到设备 {address}")
        return

    while attempt < max_attempts:
        client = BleakClient(device, disconnect_callback=disconnect_callback)
        try:
            if not client.is_connected:
                print(f"第 {attempt+1} 次尝试连接...")
                await client.connect()
                print(f"连接成功！设备地址: {client.address}")

                services = client.services
                print("\n设备服务和特征：")
                for service in services:
                    if service.uuid == SERVICE_UUID:
                        print(f"找到自定义服务: {service.uuid}")
                        for char in service.characteristics:
                            if char.uuid == CHARACTERISTIC_UUID:
                                print(f"找到目标特征: {char.uuid}")

                target_char = services.get_characteristic(CHARACTERISTIC_UUID)
                if not target_char:
                    print(f"\n未找到特征 {CHARACTERISTIC_UUID}")
                    await client.disconnect()
                    continue

                await send_command(client, "disconnect:")
                await asyncio.sleep(SLEEP_TIME) 

        except asyncio.TimeoutError:
            print("连接超时")
        except Exception as e:
            print(f"BLE操作出错: {e}")

        finally:
            if client.is_connected:
                print("\n尝试断开连接...")
                await client.disconnect()
                print("已断开")
            else:
                print("\n已处于断开状态")

        attempt += 1
        print(f"第 {attempt} 次尝试结束\n")

    print("所有尝试完成")


if __name__ == "__main__":
    asyncio.run(_connect())