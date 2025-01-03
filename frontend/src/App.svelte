<script>
	import CheckBackend from "./components/CheckBackend.svelte";
	import DnsQueryView from "./components/DnsQueryView.svelte";
	import PortScanView from "./components/PortScanView.svelte";
	import TraceRouteView from "./components/TraceRouteView.svelte";
	let componentsToRender = [];
	let targetIP = "39.156.66.10";
	targetIP = "8.134.50.24";
	targetIP = "www.baidu.com";

	function renderComponent(component) {
		componentsToRender = [...componentsToRender, component];
	}

	function getReport() {
		html2canvas(document.querySelector("#container")).then((canvas) => {
			const link = document.createElement("a"); // 创建一个a标签用于下载
			link.href = canvas.toDataURL(); // 将canvas内容转换为DataURL
			link.download = "网络侦测报告.png"; // 设置下载的文件名
			link.click(); // 模拟点击进行下载
		});
	}

	function clearReport() {
		componentsToRender = [];
	}

	let isValidIP = false;
	let isValidHostname = false;

	function validateIP(ip) {
		return /^(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$/.test(
			ip,
		);
	}

	function validateHostname(hostname) {
		return /^(?!-)[A-Za-z0-9-]{1,63}(?<!-)(\.(?!-)[A-Za-z0-9-]{1,63}(?<!-))*$/.test(
			hostname,
		);
	}

	$: isValidIP = validateIP(targetIP);
	$: isValidHostname = validateHostname(targetIP);
</script>

<div class="title-bar">网络工具</div>
<div class="container" id="container">
	<input
		type="text"
		bind:value={targetIP}
		placeholder="请输入IP地址"
		class={isValidIP || isValidHostname ? "valid-input" : "invalid-input"}
	/>

	<button
		on:click={() => renderComponent(DnsQueryView)}
		disabled={!isValidHostname}>DNS查询</button
	>
	<button
		on:click={() => renderComponent(TraceRouteView)}
		disabled={!(isValidIP || isValidHostname)}>TraceRoute</button
	>
	<button on:click={() => renderComponent(PortScanView)} disabled={!isValidIP}
		>端口扫描</button
	>

	<button on:click={getReport} disabled={componentsToRender.length == 0}
		>生成报告</button
	>

	<button on:click={clearReport}>清空</button>

	<script
		src="https://html2canvas.hertzen.com/dist/html2canvas.min.js"
	></script>
	<CheckBackend />
	<div id="report-element">
		{#each componentsToRender as component}
			<div class="fade">
				<!-- 使用 componentToRender 作为标签来渲染组件，并传递 ip 作为 prop -->
				<svelte:component this={component} {targetIP} />
			</div>
		{/each}
	</div>
</div>

<style>
	.title-bar {
		background-color: #007bff; /* 标题栏背景颜色 */
		color: white; /* 标题文字颜色 */
		padding: 10px; /* 内边距 */
		text-align: center; /* 文本居中 */
		font-size: 20px; /* 字体大小 */
	}

	.container {
		max-width: 600px;
		margin: 0 auto;
		padding: 20px;
		text-align: center;
	}

	input[type="text"] {
		width: 100%;
		padding: 10px;
		margin: 10px 0;
		border: 1px solid #ccc;
		border-radius: 4px;
	}

	button {
		padding: 10px 20px;
		margin: 5px;
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

	input[type="text"].valid-input {
		border: 2px solid green;
	}

	input[type="text"].invalid-input {
		border: 2px solid red;
	}

	button:disabled {
		background-color: #ccc;
		cursor: not-allowed;
	}
</style>
