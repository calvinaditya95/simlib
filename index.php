<html>
	<head>
		<title>BSU Cafeteria Simulation</title>
	</head>
	<body>
		<h3>BSU Cafeteria Simulation</h3>
		<h4>
			by:
			<br/>13513018 Steven Andianto
			<br/>13513077 Calvin Aditya Jonathan
		</h4>
		<hr>
		<p>
			<form method="post">
				<table>
				<tr>
					<td>Number of hot food employees:</td>
					<td><input type="text" name="hotfood"></td>
				</tr>
				<tr>
					<td>Number of specialty sandwich employees:</td>
					<td><input type="text" name="sandwich"></td>
				</tr>
				<tr>
					<td>Number of cashiers:</td>
					<td><input type="text" name="cashier"></td>
				</tr>
				</table>
				<br/><input type="submit" value="Run simulation">
			</form>
		</p>

		<?php
			if (isset($_POST['hotfood']) && isset($_POST['sandwich']) && isset($_POST['cashier'])) {
				$hotfood = $_POST['hotfood'];
				$sandwich = $_POST['sandwich'];
				$cashier = $_POST['cashier'];
				echo '<hr>';
				echo 'Simulation result for:<br/>';
				echo '<ul>';
				echo "<li>$hotfood hotfood employees</li>";
				echo "<li>$sandwich specialty sandwich employees</li>";
				echo "<li>$cashier cashiers</li>";
				echo '</ul>';
				echo '<pre>';
				passthru("./bsu 0 $hotfood $sandwich $cashier");
				echo '</pre>';
			}
		?>
	</body>
</html>