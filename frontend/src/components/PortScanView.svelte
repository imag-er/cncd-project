<script>
    import { onMount } from "svelte";
    import { fade } from "svelte/transition";

    export let targetIP = "";

    let showForm = true;

    let ip = "";
    let ports = [];
    let ws;
    let parameters = {
        start: 0,
        end: 65535,
        timeout: 1,
    };

    // 初始化WebSocket连接
    function initWebSocket() {
        showForm = false;
        ws = new WebSocket("ws://localhost:8000/ws");
        ws.onopen = () => {
            const content = {
                type: "portscan",
                target: ip,
                traceroute: {},
                dnsquery: {},
                portscan: parameters,
            };
            console.log(content);
            ws.send(JSON.stringify(content));
        };

        ws.onmessage = (event) => {
            ports = [...ports, JSON.parse(event.data)];
        };

        ws.onerror = (error) => {
            console.error("WebSocket error:", error);
        };
    }

    // 清理WebSocket连接
    function cleanupWebSocket() {
        if (ws) {
            ws.close();
        }
    }

    // 提交表单并初始化WebSocket连接
    function handleSubmit() {
        ip = targetIP;
        initWebSocket();
    }

    onMount(() => {
        return cleanupWebSocket;
    });
</script>

<h2>
    端口扫描 {ip}
    {#if showForm}
        <small> - 请填写参数并开始扫描</small>
    {:else}
        <small>- 端口扫描结果</small>
        <br />
        <small>
            范围 {parameters.start} - {parameters.end} (超时时间: {parameters.timeout}s)
        </small>
    {/if}
</h2>

{#if showForm}
    <form
        on:submit|preventDefault={handleSubmit}
        style="display: flex; flex-direction: column; height: 100%;"
    >
        <div
            style="display: grid; grid-template-columns: repeat(3, 1fr); gap: 20px; align-items: center; justify-content: center; flex-grow: 1; width:100%;"
        >
            {#each Object.keys(parameters) as key}
                <div class="input-container">
                    <label for={key}>{key}:</label>
                    <input
                        type="number"
                        id={key}
                        bind:value={parameters[key]}
                        required
                    />
                </div>
            {/each}
        </div>
        <button type="submit">开始扫描</button>
    </form>
{/if}

<ul class="port-list">
    {#each ports as port (port.port)}
        <li class="port-item" in:fade>
            <p>
                <strong>Port:</strong>{port.port} -
                <strong>Status: Open</strong>
            </p>
        </li>
    {/each}
</ul>

<style>
    .input-container {
        margin-bottom: 10px;
        text-align: center;
    }

    label {
        display: block;
        margin-bottom: 5px;
        text-align: center;
    }

    input[type="number"] {
        width: 100%;
        padding: 8px;
        margin: 0 auto;
        display: block;
        border: 1px solid #ccc;
        border-radius: 4px;
    }

    button {
        padding: 10px 20px;
        border: none;
        border-radius: 4px;
        background-color: #007bff;
        color: white;
        cursor: pointer;
        transition: background-color 0.3s ease;
    }

    button:hover {
        background-color: #0056b3;
    }

    .port-list {
        list-style: none;
        padding: 0;
    }

    .port-item {
        border-left: 5px solid green;
        margin: 10px 0;
        padding: 10px;
        background-color: #f4f4f4;
    }

    .port-item:hover {
        background-color: #e9e9e9;
    }
</style>
