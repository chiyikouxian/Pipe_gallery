
> ## Archived Historical Reference (Not Current Target)
> This document records a previously explored HTTP external-server direction.
> It is **not** part of the current active implementation baseline.
> Current baseline remains the restored embedded direction centered on existing LoRa/Modbus/current cloud code paths.
> Do not treat this file as active implementation guidance unless a new approved change explicitly re-activates it.


| 服务器协议    | HTTP POST                                 |
| -------- | ----------------------------------------- |
| 数据格式     | JSON                                      |
| API 路径   | `/PipeLines`                              |
| 认证方式     | JWT Token，放在请求头 `token` 中（不带 `Bearer` 前缀） |
| 登录接口     | `POST /login`                             |
| 登录字段     | 见下方                                       |
| 甲烷浓度字段   | `concentration`（单位：百分比）                   |
| 其他可选上报字段 | 见下方                                       |

---



## 登录接口

| 项目        | 内容                                              |
| --------- | ----------------------------------------------- |
| **URL**   | `http://112.124.59.215:8080/login`              |
| **请求方式**  | `POST`                                          |
| **协议**    | HTTP（非 HTTPS，ESP8266 可直接连接）                     |
| **请求头**   | `Content-Type: application/json`                |
| **请求体字段** | `username`（字符串，必填）`password`（字符串，必填）            |
| **请求体示例** | `{"username":"jane_smith","password":"123456"}` |

### 登录成功响应

- **状态码**：`200 OK`
- **响应体格式**：
  
  ```json
  {
    "code": 200,
    "msg": "success",
    "data": {
        "userId": 2,
        "username": "jane_smith",
        "email": "jane.smith@example.com",
        "phone": "0987654321",
        "name": "史密斯",
        "role": "ADMIN",
        "token": "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9...",
        "createdAt": "2024-07-23T03:39:43.000+00:00",
        "updatedAt": "2024-11-10T05:27:15.000+00:00"
    }
  }
  ```
- **Token 字段路径**：`data.token`（即从响应中通过 `data.token` 获取 JWT 字符串）

### 登录失败响应

- **状态码**：`200 OK`（业务错误仍返回 200，通过 `code` 字段判断）
- **响应体示例**：
  
  ```json
  {
    "code": 500,
    "msg": "用户名或密码错误",
    "data": null
  }
  ```

---

## 可用测试账号

| 用户名          | 密码         | 角色         |
| ------------ | ---------- | ---------- |
| `jane_smith` | `123456`   | ADMIN（管理员） |
| `john_doe`   | `john_doe` | USER（普通用户） |

---

## 气体浓度数据上报接口（/PipeLines）

### 3.1 接口基本信息

| 项目       | 内容                                                                   |
| -------- | -------------------------------------------------------------------- |
| **URL**  | `http://112.124.59.215:8080/PipeLines`                               |
| **请求方式** | `POST`                                                               |
| **认证方式** | 请求头携带 `token: <登录获得的JWT>`（**不带** `Bearer` 前缀）                        |
| **请求头**  | `Content-Type: application/json`<br>`token: eyJhbGciOiJIUzI1NiJ9...` |

### 3.2 请求体字段（完整列表）

| 字段名                     | 类型      | 是否必填  | 说明       | 可选值/单位                            |
| ----------------------- | ------- | ----- | -------- | --------------------------------- |
| `pipelineCategory`      | String  | **是** | 管线类别     | `"电力"`、`"燃气"`、`"给排水"`             |
| `currentPipelineStatus` | String  | **是** | 当前状态     | `"正常"`、`"故障"`、`"正在维修"`、`"未知"`     |
| `pipeGallery_id`        | Integer | **是** | 所属管廊 ID  | 需预先存在，可通过 `GET /PipeGalleries` 查询 |
| `temperature`           | Double  | 否     | 温度       | 单位：摄氏度                            |
| `humidity`              | Double  | 否     | 湿度       | 单位：百分比                            |
| `concentration`         | Double  | 否     | **气体浓度** | 单位：百分比（注释），需与业务确认实际单位             |
| `cracks`                | Double  | 否     | 裂缝长度     | 单位：毫米                             |
| `deformation`           | Double  | 否     | 变形量      | 单位：百分比                            |
| `corrosion`             | Double  | 否     | 腐蚀深度     | 单位：毫米                             |

### 3.3 请求体示例（JSON）

```json
{
    "pipelineCategory": "燃气",
    "currentPipelineStatus": "正常",
    "pipeGallery_id": 6,
    "temperature": 25.5,
    "humidity": 60.0,
    "concentration": 2.3,
    "cracks": 0.0,
    "deformation": 0.1,
    "corrosion": 0.05
}
```

### 3.4 成功响应

- **状态码**：`200 OK`
- **响应体**：
  
  ```json
  {
    "code": 200,
    "msg": "success",
    "data": "PipeLine added successfully"
  }
  ```

### 3.5 失败响应示例（Token 无效或过期）

- **状态码**：`200 OK`
- **响应体**：
  
  ```json
  {
    "code": 401,
    "msg": "请登录",
    "data": null
  }
  ```
  
  > **注意**：Token 有效期为 **1 小时**，过期后需重新调用 `/login` 获取新 Token。

---

## 四、已有管廊 ID 列表（供 `pipeGallery_id` 使用）

| ID  | 位置   | 所属分区    |
| --- | ---- | ------- |
| 1   | 大学路  | 成都理工大学  |
| 2   | 环湾路  | 宜宾学院    |
| 3   | 观山路  | 四川外国语学院 |
| 4   | 峥嵘路  | 成都工业学院  |
| 5   | 龙兴路  | 成都理工大学  |
| 6   | 博学二街 | 成都理工大学  |
| 7   | 博学一街 | 宜宾学院    |
| 8   | 博学三街 | 四川外国语大学 |
| 9   | 文博路  | 宜宾学院    |
| 10  | 长翠路  | 成都理工大学  |
| 11  | 白塔路  | 西华大学    |

---

### 其他： 管廊监测设备（PipelineMonitoringDevice）字段一览

| 字段名（JSON key）       | 类型      | 是否必填 | 说明                       |
| ------------------- | ------- | ---- | ------------------------ |
| `classification`    | String  | 否    | 所属分区                     |
| `deviceName`        | String  | 是    | 设备名称                     |
| `manufacturer`      | String  | 否    | 制造商                      |
| `type`              | String  | 是    | 设备类型（如"气体检测仪"）           |
| `maxUseYears`       | Integer | 否    | 最长使用年限                   |
| `remainingUseYears` | Integer | 否    | 剩余使用年限                   |
| `status`            | String  | 是    | 设备状态（如"运行中"、"维修中"、"已停用"） |
| `street`            | String  | 否    | 所属街道/路段                  |
