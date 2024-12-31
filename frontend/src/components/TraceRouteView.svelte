<script lang="ts">
    import { onMount } from "svelte";

    export let targetIP: string;
    let ip = "";
    let arrived: boolean = false;
    let ws: WebSocket;
    let tracerouteData = [];

    let parameters = {
        first_ttl: 1,
        max_ttl: 10,
        wait_time: 3,
        max_timeout: 3,
        nqueries: 3,

        packet_size: 60,
    };
    let parameterKeys = Object.keys(parameters);
    let showForm = true;

    const startTraceroute = () => {
        showForm = false;
        ip = targetIP;
        console.log("ip", targetIP);
        ws = new WebSocket("ws://localhost:8000/ws");
        ws.onopen = () => {
            let content = JSON.stringify({
                type: "traceroute",
                target: ip,
                traceroute:parameters,
                dnsquery:{},
                portscan:{}
            });

            console.log(content);
            ws.send(content);
        };

        ws.onmessage = (event) => {
            console.log(event.data);
            tracerouteData = [...tracerouteData, JSON.parse(event.data)];
        };

        ws.onerror = (event) => {
            console.log(event);
        };

        ws.onclose = () => console.log("Connection closed");
    };

    onMount(() => {
        return () => ws?.close();
    });

    $: arrived = tracerouteData.some((item) => item.address === ip);
</script>

<h2>
    路由跟踪 {ip}
    {#if showForm}
        <small> - 请填写参数并开始跟踪</small>
    {:else }
        {#if !arrived}
            <small> - 正在跟踪...</small>
        {:else}
            <small> - 已到达目标</small>
        {/if}
        <br />
        <small>
            包大小:{parameters.packet_size}B    TTL:{parameters.first_ttl} - {parameters.max_ttl}    (最大等待时间: {parameters.wait_time}s 超时时间: {parameters.max_timeout}s)
        </small>
    {/if}
</h2>
{#if showForm}
    <form
        on:submit|preventDefault={startTraceroute}
        style="display: flex; flex-direction: column; height: 100%;"
    >
        <div
            style="display: grid; grid-template-columns: repeat(3, 1fr); align-items: center; justify-content: center; flex-grow: 1; width:100%; "
        >
            {#each parameterKeys as key}
                <div
                    style="display: flex; flex-direction: column; align-items: center; width:100%;"
                >
                    <label for={key} >{key}</label>
                    <input
                        type="number"
                        id={key}
                        bind:value={parameters[key]}
                        style="padding: 8px; border: 1px solid #ccc; border-radius: 4px;"
                    />
                </div>
            {/each}
        </div>
        <div style="display: flex; justify-content: center; margin-top: 20px; width:100%">
            <button
                type="submit"
                style="padding: 10px 20px; background-color: #007bff; color: white; border: none; border-radius: 4px; cursor: pointer;"
                on:click={()=>{console.log(parameters)}}
                >确认并开始跟踪</button
            >
        </div>
    </form>
{:else}
    <div class="traceroute-container">
        <ul class="traceroute-list">
            {#each tracerouteData as item}
                <li
                    class={item.address === ip
                        ? "traceroute-item-arrived"
                        : "traceroute-item-no-arrive"}
                >
                    <strong>{item.hop}</strong>
                    <strong>{item.address}</strong>
                    {#if item.timeout !== null}
                        {#each item.timeout as timeout}
                            <span class="delay">{timeout} ms</span>
                        {/each}
                    {:else}
                        <span class="delay">请求超时</span>
                    {/if}
                </li>
            {/each}
        </ul>
    </div>
{/if}

<style>
    form {
        display: flex;
        flex-direction: column;
        align-items: flex-start;
        margin: 20px;
    }

    form div {
        margin-bottom: 10px;
    }

    label {
        margin-right: 10px;
    }

    input[type="number"] {
        padding: 8px;
        margin-left: 5px;
        border: 1px solid #ccc;
        border-radius: 4px;
        transition: border-color 0.3s ease-in-out;
    }

    input[type="number"]:focus {
        border-color: #66afe9;
        outline: none;
    }

    button {
        padding: 10px 20px;
        background-color: #007bff;
        color: white;
        border: none;
        border-radius: 4px;
        cursor: pointer;
        transition: background-color 0.3s ease-in-out;
    }

    button:hover {
        background-color: #0056b3;
    }

    .traceroute-list {
        list-style: none;
        padding: 0;
    }

    .traceroute-item-no-arrive {
        margin: 10px 0;
        padding: 10px;
        background-color: #f4f4f4;
        border-left: 5px solid #aaa;
        animation: fadeIn 0.5s;
    }
    .traceroute-item-arrived {
        margin: 10px 0;
        padding: 10px;
        background-color: #97dfa3;
        border-left: 5px solid #1a7e51;
        animation: fadeIn 0.5s;
    }

    .delay {
        margin-left: 10px;
        color: #666;
    }

    @keyframes fadeIn {
        from {
            opacity: 0;
        }
        to {
            opacity: 1;
        }
    }

    form div {
        margin-bottom: 0; /* 移除底部外边距，因为网格布局会自动处理间距 */
    }

    form div div {
        margin-bottom: 10px;
    }
</style>
