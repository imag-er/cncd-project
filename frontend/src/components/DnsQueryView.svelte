<script>
    import { onMount } from "svelte";
    export let targetIP = "";
    let ip = targetIP;
    let ws;
    let queries = [];
    let parameters = {};

    onMount(() => {
        ws = new WebSocket("ws://localhost:8000/ws");
        ws.onopen = () => {
            const content = {
                type: "dnsquery",
                target: ip,
                traceroute: {},
                dnsquery: parameters,
                portscan: {},
            };
            console.log(content);
            ws.send(JSON.stringify(content));
        };
        ws.onmessage = function (event) {
            console.log(event.data);
            console.log("parsing");
            const data = JSON.parse(JSON.parse(event.data));
            queries = [...queries, data];
            console.log(queries);
        };

        return () => {
            ws.close();
        };
    });
</script>

<h2>
    DNS查询 {ip}
    {#if queries.length === 0}
        <small> - 正在查询...</small>
    {/if}
</h2>
<ul class="query-list">
    {#each queries as query}
        {#if query.type == "query"}
            <li class="query-item">
                {query.ip} Host: {query.response} - Type: {query["record-type"]}
            </li>
        {:else if query.type == "result"}
            {#each query.results as result}
                <li class="result-item">
                    Result: {result.ip}  TTL:{result.TTL}
                </li>
            {/each}
        {/if}
    {/each}
</ul>

<style>
    .query-list {
        list-style: none;
        padding: 0;
    }

    .query-item {
        margin: 10px 0;
        padding: 10px;
        background-color: #f4f4f4;
        border-left: 5px solid #40ab55;
        animation: fadeIn 0.5s;
    }
    .result-item {
        margin: 10px 0;
        padding: 10px;
        background-color: #f4f4f4;
        border-left: 5px solid #0f5fa0;
        animation: fadeIn 0.5s;
    }

    @keyframes fadeIn {
        from {
            opacity: 0;
        }
        to {
            opacity: 1;
        }
    }
</style>
